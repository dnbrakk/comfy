/**
 * Comfy
 *
 *  Copyright 2019 by Wolfish <wolfish@airmail.cc>
 *  https://wolfish.neocities.org/soft/comfy/
 *
 *  Licensed under the GPL v2.0 only.
 */
#include "netops.h"
#include "fileops.h"
#include "widgetman.h"
#include "imgman.h"

struct GoogleCaptchaChallengeInfo
{
    std::string id;
    std::string payload_url;
    std::string description;
};

static bool google_captcha_response_extract_id(const std::string& html, std::string& result)
{
    size_t value_index = html.find("value=\"");
    if(value_index == std::string::npos)
        return false;

    size_t value_begin = value_index + 7;
    size_t value_end = html.find('"', value_begin);
    if(value_end == std::string::npos)
        return false;

    size_t value_length = value_end - value_begin;
    // The id is also only valid if it's in base64, but it might be overkill to verify if it's base64 here
    if(value_length < 300)
        return false;

    result = html.substr(value_begin, value_length);
    return true;
}

static bool google_captcha_response_extract_goal_description(const std::string& html, std::string& result)
{
    size_t goal_description_begin = html.find("rc-imageselect-desc-no-canonical");
    if(goal_description_begin == std::string::npos)
        return false;

    goal_description_begin = html.find('>', goal_description_begin);
    if(goal_description_begin == std::string::npos)
        return false;

    goal_description_begin += 1;

    size_t goal_description_end = html.find("</div", goal_description_begin);
    if(goal_description_end == std::string::npos)
        return false;

    // The goal description with google captcha right now is "Select all images with <strong>subject</strong>".
    // TODO: Should the subject be extracted, so bold styling can be applied to it?
    result = html.substr(goal_description_begin, goal_description_end - goal_description_begin);
    replace_substr(result, "<strong>", "");
    replace_substr(result, "</strong>", "");
    return true;
}

static std::experimental::optional<GoogleCaptchaChallengeInfo> google_captcha_parse_request_challenge_response(const std::string& api_key, const std::string& html_source)
{
    GoogleCaptchaChallengeInfo result;
    if(!google_captcha_response_extract_id(html_source, result.id))
        return std::experimental::nullopt;
    result.payload_url = "https://www.google.com/recaptcha/api2/payload?c=" + result.id + "&k=" + api_key;
    if(!google_captcha_response_extract_goal_description(html_source, result.description))
        return std::experimental::nullopt;
    return result;
}

// Note: This assumes strings (quoted data) in html tags dont contain '<' or '>'
static std::string strip_html_tags(const std::string& text)
{
    std::string result;
    size_t index = 0;
    while(true)
    {
        size_t tag_start_index = text.find('<', index);
        if(tag_start_index == std::string::npos)
        {
            result.append(text.begin() + index, text.end());
            break;
        }

        result.append(text.begin() + index, text.begin() + tag_start_index);
        
        size_t tag_end_index = text.find('>', tag_start_index + 1);
        if(tag_end_index == std::string::npos)
            break;
        index = tag_end_index + 1;
    }
    return result;
}

static std::experimental::optional<std::string> google_captcha_parse_submit_solution_response(const std::string& html_source)
{
    size_t start_index = html_source.find("\"fbc-verification-token\">");
    if(start_index == std::string::npos)
        return std::experimental::nullopt;

    start_index += 25;
    size_t end_index = html_source.find("</", start_index);
    if(end_index == std::string::npos)
        return std::experimental::nullopt;

    return strip_html_tags(html_source.substr(start_index, end_index - start_index));
}

// thread safe queues
threadsafe_queue<data_4chan> NetOps::queue__4chan_json;


void NetOps::init()
{
    curl_global_init(CURL_GLOBAL_ALL);
}


void NetOps::shutdown()
{
    curl_global_cleanup();
}


  //////////////////
 // get requests //
//////////////////

void NetOps::http_get__4chan_json(std::string url, std::string wgt_id, bool b_steal_focus, long last_fetch_time, std::string job_pool_id, bool b_push_to_front)
{
    THREAD_MAN.enqueue_job(
        std::bind(curl__get_4chan_json, url, wgt_id, last_fetch_time, b_steal_focus),
        job_pool_id,
        b_push_to_front);
}


void NetOps::http_get__image(http_image_req& req, std::string job_pool_id, bool b_push_to_front)
{
    if (DISPLAY_IMAGES && req.is_valid())
    {
        THREAD_MAN.enqueue_job(
            std::bind(curl__get_image, req),
            job_pool_id,
            b_push_to_front);
    }
}

void NetOps::http_get__google_captcha(const std::string& api_key, const std::string& referer, RequestGoogleCaptchaCallback callback, const std::string& job_pool_id, bool b_push_to_front)
{
    assert(callback);
    THREAD_MAN.enqueue_job(
        std::bind(curl__get_google_captcha, api_key, referer, std::move(callback)),
        job_pool_id,
        b_push_to_front);
}

void NetOps::http_post__google_captcha_solution(const std::string& api_key, const std::string& captcha_id, GoogleCaptchaSolution solution, SubmitGoogleCaptchaSolutionCallback callback, std::string job_pool_id, bool b_push_to_front)
{
    assert(callback);
    THREAD_MAN.enqueue_job(
        std::bind(curl__post_google_captcha_solution, api_key, captcha_id, std::move(solution), std::move(callback)),
        job_pool_id,
        b_push_to_front);
}


  ////////////////////
 // curl launching //
////////////////////

void NetOps::curl__get_4chan_json(std::string url, std::string wgt_id, long last_fetch_time, bool b_steal_focus)
{
    data_4chan chan_data(url, wgt_id);
    // error: invalid url
    if (!chan_data.url_is_valid())
    {
        chan_data.error_type = e_error_type::et_invalid_url;
        queue__4chan_json.push(chan_data);

        return;
    }

    std::stringstream out_buf;

    auto handle = curl_easy_init(); 
    curl_easy_setopt(handle, CURLOPT_URL, chan_data.parser.url.c_str());
    // fail if e.g. http error 404
    curl_easy_setopt(handle, CURLOPT_FAILONERROR, true);

    // if modified since
    curl_easy_setopt(handle, CURLOPT_TIMEVALUE, last_fetch_time);
    curl_easy_setopt(handle, CURLOPT_TIMECONDITION, CURL_TIMECOND_IFMODSINCE);

    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, curl_write_data);
    // set pointer that is passed to curl write function as fourth param
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, static_cast<void*>(&out_buf)); 
    auto success = curl_easy_perform(handle);

    long code;
    curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &code);
    chan_data.http_response = code;
    if (code == 404)
    {
        chan_data.error_type = et_http_404;
    }

    if (success == CURLE_OK)
    {
        // not modified since last_fetch_time
        long unmet;
        curl_easy_getinfo(handle, CURLINFO_CONDITION_UNMET, &unmet);
        if (unmet == 1)
        {
            chan_data.error_type = et_not_mod_since;
        }
        // all is well, parse json
        else
        {
            if (!chan_data.parse_json(out_buf.str().c_str()))
            {
                // json parse error
                chan_data.error_type = et_json_parse;
            }
            else
            {
                std::string f_path = chan_data.file_path;
                std::string f_name =
                    std::to_string(time_now_ms().count()) + ".json";

                // save json file to disk
                switch (chan_data.parser.pagetype)
                {
                    case pt_boards_list         :
                        f_name = "boards_list.json";
                        break;
                    case pt_board_page          :
                        break;
                    case pt_board_catalog       :
                        f_name = "catalog.json";
                        break;
                    case pt_board_threads       :
                        break;
                    case pt_board_archive       :
                        break;
                    case pt_thread              :
                        f_name = "thread.json";
                        break;
                }

                if (!f_path.empty() && !f_name.empty())
                {
                    FileOps::write_file(f_path, f_name, out_buf.str().c_str(), out_buf.str().length());
                }
            }

            chan_data.fetch_time = time_now_s().count();
        }
    }

    curl_easy_cleanup(handle);
    chan_data.curl_result = success;
    chan_data.b_steal_focus = b_steal_focus;

    // queue data
    queue__4chan_json.push(chan_data);
}


void NetOps::curl__get_image(http_image_req& req)
{
    // error: invalid url
    if (!req.url_is_valid())
    {
        req.error_type = e_error_type::et_invalid_url;
        // TODO: send error message to WIDGET_MAN

        return;
    }

    std::stringstream out_buf;

    auto handle = curl_easy_init(); 
    curl_easy_setopt(handle, CURLOPT_URL, req.parser.url.c_str());
    // fail if e.g. http error 404
    curl_easy_setopt(handle, CURLOPT_FAILONERROR, true);
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, curl_write_data);
    // set pointer that is passed to curl write function as fourth param
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, static_cast<void*>(&out_buf)); 
    auto success = curl_easy_perform(handle);

    long code;
    curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &code);
    req.http_response = code;
    if (code == 404)
    {
        req.error_type = et_http_404;
    }

    if (success == CURLE_OK)
    {
        FileOps::write_file(req.get_file_path(), req.get_file_name(), out_buf.str().c_str(), out_buf.str().length());

        // load image and dispatch img_packet to dest widget
        img_packet pac(
            req.size,
            req.thread_key,
            req.post_key,
            req.parser,
            req.get_file_path());

        IMG_MAN.threaded_load_img_from_disk(pac);
    }

    // TODO: send http_image_req back to widget_man for error processing

    curl_easy_cleanup(handle);
    req.curl_result = success;
}

void NetOps::curl__get_google_captcha(std::string api_key, std::string referer, RequestGoogleCaptchaCallback callback)
{
    assert(callback);
    fprintf(stderr, "curl__get_google_captcha, api key: %s\n", api_key.c_str());
    // TODO: Do not request need google captcha if it the last time it was requested was less than 2 minutes ago

    std::stringstream out_buf;
    std::string captcha_url = "https://www.google.com/recaptcha/api/fallback?k=" + api_key;

    auto handle = curl_easy_init(); 
    curl_easy_setopt(handle, CURLOPT_URL, captcha_url.c_str());
    // fail if e.g. http error 404
    curl_easy_setopt(handle, CURLOPT_FAILONERROR, true);

    curl_easy_setopt(handle, CURLOPT_REFERER, referer.c_str());

    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, curl_write_data);
    // set pointer that is passed to curl write function as fourth param
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, static_cast<void*>(&out_buf)); 
    auto success = curl_easy_perform(handle);

    long code;
    curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &code);
    fprintf(stderr, "curl__get_google_captcha, response code %ld, success: %s\n", code, success == CURLE_OK ? "yes" : "no");
    if (code == 404)
        callback(std::experimental::nullopt);
    else if (success == CURLE_OK)
    {
        std::experimental::optional<GoogleCaptchaChallengeInfo> captcha_info = google_captcha_parse_request_challenge_response(api_key, out_buf.str());
        if (!captcha_info)
        {
            callback(std::experimental::nullopt);
        }
        else
        {
            fprintf(stderr, "curl__get_google_captcha, successfully parsed html!, |%s|, |%s|, |%s|\n",
                captcha_info->id.c_str(), captcha_info->payload_url.c_str(), captcha_info->description.c_str());
            vector2d size(300, 300);
            http_image_req req(
                captcha_info->payload_url,
                size,
                "",
                "",
                -1,
                TEMP_DIR,
                api_key + "_google_captcha.jpeg"
            );
            // TODO: Verify if this is safe, since the image will always have the same file path. Any race condition issues?
            curl__get_image(req);
            if(req.curl_result == CURLE_OK)
            {
                GoogleCaptchaChallenge challenge;
                challenge.id = captcha_info->id;
                challenge.description = captcha_info->description;
                img_packet pac(
                    req.size,
                    req.thread_key,
                    req.post_key,
                    req.parser,
                    req.get_file_path());
                challenge.image = std::move(pac);
                callback(std::move(challenge));
            }
            else
            {
                callback(std::experimental::nullopt);
            }
        }
    }

    fflush(stderr);
    curl_easy_cleanup(handle);
}

void NetOps::curl__post_google_captcha_solution(std::string api_key, std::string captcha_id, GoogleCaptchaSolution solution, SubmitGoogleCaptchaSolutionCallback callback)
{
    assert(callback);
    fprintf(stderr, "curl__post_google_captcha_solution, api key: %s\n", api_key.c_str());

    std::stringstream out_buf;
    std::string captcha_url = "https://www.google.com/recaptcha/api/fallback?k=" + api_key;
    std::string referer = captcha_url;
    captcha_url += "&c=" + captcha_id + "&" + solution.build_http_post_data();

    auto handle = curl_easy_init(); 
    curl_easy_setopt(handle, CURLOPT_URL, captcha_url.c_str());
    // fail if e.g. http error 404
    curl_easy_setopt(handle, CURLOPT_FAILONERROR, true);

    curl_easy_setopt(handle, CURLOPT_REFERER, referer.c_str());

    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, curl_write_data);
    // set pointer that is passed to curl write function as fourth param
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, static_cast<void*>(&out_buf)); 
    auto success = curl_easy_perform(handle);

    long code;
    curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &code);
    fprintf(stderr, "curl__post_google_captcha_solution, response code %ld, success: %s\n", code, success == CURLE_OK ? "yes" : "no");
    if (code == 404)
        callback(std::experimental::nullopt);
    else if (success == CURLE_OK)
    {
        std::experimental::optional<std::string> solved_captcha_id = google_captcha_parse_submit_solution_response(out_buf.str());
        if (!solved_captcha_id)
        {
            callback(std::experimental::nullopt);
        }
        else
        {
            fprintf(stderr, "curl__post_google_captcha_solution, successfully parsed html!, |%s|\n", solved_captcha_id->c_str());
            callback(std::move(solved_captcha_id));
        }
    }

    fflush(stderr);
    curl_easy_cleanup(handle);
}


  //////////////////////////
 // curl data processing //
//////////////////////////

size_t NetOps::curl_write_data(char* buffer, size_t size, size_t nmemb, void* out)
{
    // if the function returns less than this value,
    // curl considers it an error and aborts the download
    size_t real_size = size * nmemb;

    std::stringstream* out_buf = static_cast<std::stringstream*>(out);
    for (int i = 0; i < real_size; ++i)
    {
        out_buf->put(*buffer);
        buffer += sizeof(char);
    }

    return real_size;
}

