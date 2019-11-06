/**
 * Comfy
 *
 *  Copyright 2019 by Wolfish <wolfish@airmail.cc>
 *  https://wolfish.neocities.org/soft/comfy/
 *
 *  Licensed under the GPL v2.0 only.
 */
#pragma once
#include "comfy.h"
#include "threadman.h"
#include <curl/curl.h>

using namespace HTML_Utils;


enum e_error_type
{
    et_NONE,
    et_invalid_url,
    et_http_404,
    et_json_parse,
    et_not_mod_since,
};


// http get requests

struct http_request
{
    http_request()
    : parser(url_parser())
    , curl_result(-1)
    , http_response(-1)
    , error_type(e_error_type::et_NONE)
    , file_path("")
    , fetch_time(0)
    {
    }
    

    url_parser parser;
    int curl_result;
    int http_response;
    e_error_type error_type;
    std::string file_path;
    long fetch_time; // seconds, time file was fetched


    void set_url(std::string url, std::string thread_num_str = "")
    {
        parser = url_parser(url);
        file_path = get_file_save_dir(parser, thread_num_str);
    }

    virtual bool url_is_valid() const
    {
        return parser.is_valid();
    }

    virtual bool is_valid() const
    {
        return url_is_valid() && error_type == e_error_type::et_NONE;
    }
};


struct http_image_req : http_request
{
    http_image_req()
    : http_request()
    , size(vector2d())
    , thread_key("")
    , post_key(-1)
    {}

    http_image_req(
        std::string url,
        vector2d _size,
        std::string _thread_key,
        std::string thread_num_str,
        int _post_key,
        // file path and file name can be overridden
        std::string _custom_file_path = "",
        std::string _custom_file_name = ""
    )
    : size(_size)
    , thread_key(_thread_key)
    , post_key(_post_key)
    , custom_file_path(_custom_file_path)
    , custom_file_name(_custom_file_name)
    {
        set_url(url, thread_num_str);
    }

    // in term cells
    vector2d size;
    std::string thread_key;
    int post_key;
    std::string custom_file_path;
    std::string custom_file_name;

    std::string get_file_path()
    {
        if (custom_file_path.empty())
        {
            return file_path;
        }
        else
        {
            return custom_file_path;
        }
    }

    std::string get_file_name()
    {
        if (custom_file_name.empty())
        {
            return parser.file_name;
        }
        else
        {
            return custom_file_name;
        }
    }

    bool is_video() const
    {
        return
            parser.file_ext.compare("mp4") == 0 ||
            parser.file_ext.compare("webm") == 0 ||
            parser.file_ext.compare("mpeg") == 0 ||
            parser.file_ext.compare("mpg") == 0 ||
            parser.file_ext.compare("MP4") == 0 ||
            parser.file_ext.compare("WEBM") == 0 ||
            parser.file_ext.compare("MPEG") == 0 ||
            parser.file_ext.compare("MPG") == 0;
    }

};


struct data_4chan : http_request
{
    data_4chan()
    : http_request()
    , wgt_id("")
    , b_steal_focus(false)
    {
        page_data = std::make_shared<imageboard::page_data>();
    }

    data_4chan(std::string _url, std::string _wgt_id = "")
    : http_request()
    , wgt_id(_wgt_id)
    , b_steal_focus(false)
    {
        set_url(_url);
        // widget id defaults to url
        if (wgt_id.empty())
        {
            wgt_id = parser.url;
        }
        page_data = std::make_shared<imageboard::page_data>();
        page_data->url = parser.url;
        page_data->board = parser.board;
    }


    std::string wgt_id;
    std::shared_ptr<imageboard::page_data> page_data;
    bool b_steal_focus;


    // returns true if there are no parse errors
    bool parse_json(const char* json)
    {
        if (!page_data) return false;

        switch (parser.pagetype)
        {
            case pt_thread          :
                return JSON_Utils::parse_4chan_thread(
                    json, *page_data.get());

            case pt_boards_list     :
                return JSON_Utils::parse_4chan_boards_list(
                    json, *page_data.get());

            case pt_board_catalog   :
                return JSON_Utils::parse_4chan_catalog(
                    json, *page_data.get());
        }

        return false;
    }

    virtual bool url_is_valid() const override
    {
        return parser.is_valid() && parser.pagetype != e_page_type::pt_UNKNOWN;
    }

    virtual bool is_valid() const override
    {
        return page_data &&
                url_is_valid() && error_type == e_error_type::et_NONE;
    }
};


class NetOps
{

public:

    static void init();
    static void shutdown();

    // thread safe queues
    static threadsafe_queue<data_4chan> queue__4chan_json;

    // get requests
    static void http_get__4chan_json(std::string url, std::string wgt_id = "", bool b_steal_focus = false, long last_fetch_time = 0, std::string job_pool_id = DEFAULT_JOB_POOL_ID, bool b_push_to_front = true);
    static void http_get__image(http_image_req& req, std::string job_pool_id = DEFAULT_JOB_POOL_ID, bool b_push_to_front = true);

    // curl launching
    static void curl__get_4chan_json(std::string url, std::string wgt_id, long last_fetch_time, bool b_steal_focus);
    static void curl__get_image(http_image_req req);

    // curl data processing
    static size_t curl_write_data(char* buffer, size_t size, size_t nmemb, void* out); 

};

