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

void NetOps::http_get__4chan_json(std::string url, bool b_steal_focus, long last_fetch_time, std::string job_pool_id, bool b_push_to_front)
{
    THREAD_MAN.enqueue_job(
        std::bind(curl__get_4chan_json, url, last_fetch_time, b_steal_focus),
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


  ////////////////////
 // curl launching //
////////////////////

void NetOps::curl__get_4chan_json(std::string url, long last_fetch_time, bool b_steal_focus)
{
    data_4chan chan_data(url);
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


void NetOps::curl__get_image(http_image_req req)
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

