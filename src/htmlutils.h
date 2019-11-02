/**
 * Comfy
 *
 *  Copyright 2019 by Wolfish <wolfish@airmail.cc>
 *  https://wolfish.neocities.org/soft/comfy/
 *
 *  Licensed under the GPL v2.0 only.
 */
#pragma once
#include "stringutils.h"


namespace HTML_Utils
{
    using namespace std;


    enum e_website
    {
        ws_UNKNOWN,
        ws_4chan,
    };

    enum e_page_type
    {
        pt_UNKNOWN,
        pt_boards_list,
        pt_board_page,
        pt_board_catalog,
        pt_board_threads,
        pt_board_archive,
        pt_thread,
        pt_image,
        pt_image_thumbnail,
        pt_image_flag,
        pt_image_spoiler,
    };


    struct url_parser
    {
        url_parser()
        : url("")
        , address("")
        , document_path("")
        , board("")
        , thread_num_str("")
        , file_name("")
        , file_ext("")
        , pagetype(pt_UNKNOWN)
        , website(ws_UNKNOWN)
        {}

        url_parser(string _url)
        : url(_url)
        , address("")
        , document_path("")
        , board("")
        , thread_num_str("")
        , file_name("")
        , file_ext("")
        , pagetype(pt_UNKNOWN)
        , website(ws_UNKNOWN)
        {
            parse();
        }
            

        string url;
        string address;
        string document_path;
        string board;
        string thread_num_str;
        string file_name;
        string file_ext;
        e_page_type pagetype;
        e_website website;


        bool is_valid() const
        {
            return !url.empty(); 
        }

        virtual void parse()
        {
            /*     4chan API URLs
             *
             *     User images: https://i.4cdn.org/[board]/[4chan image ID].[file extension]
             *     Thumbnails: https://i.4cdn.org/[board]/[4chan image ID]s.jpg The s after the image ID indicates the file is a thumbnail.
             *     Country flags: https://s.4cdn.org/image/country/[country code].gif
             *     /pol/ special flags: https://s.4cdn.org/image/country/troll/[country code].gif
             *     Spoiler image: https://s.4cdn.org/image/spoiler.png
             *     Custom spoilers: https://s.4cdn.org/image/spoiler-[board][1-5].png
             *
             *     boards list: https://a.4cdn.org/boards.json
             *     board page (index): https://a.4cdn.org/po/2.json
             *     board catalog: https://a.4cdn.org/po/catalog.json
             *     board threads: https://a.4cdn.org/po/threads.json
             *     board archive: https://a.4cdn.org/po/archive.json
             *     thread: https://a.4cdn.org/po/thread/570368.json
             *
             */

            // remove http:// or https://
            vector<string> str_tests;
            str_tests.emplace_back("http://");
            str_tests.emplace_back("https://");

            for (auto& s : str_tests)
            {
                if (url.length() >= s.length())
                {
                    if (url.substr(0, s.length()).find(s, 0) != string::npos)
                    {
                        url = url.substr(s.length(), url.length());
                        break;
                    }
                }
            }
        
            // is valid URL
            str_tests.clear();
            str_tests.emplace_back("4chan");
            str_tests.emplace_back("4channel");
            str_tests.emplace_back("4chan.org");
            str_tests.emplace_back("4channel.org");
            str_tests.emplace_back("boards.4chan.org");
            str_tests.emplace_back("boards.4channel.org");
            str_tests.emplace_back("a.4cdn.org");
            str_tests.emplace_back("i.4cdn.org");
            str_tests.emplace_back("s.4cdn.org");

            bool b_valid = false;

            for (auto& s : str_tests)
            {
                if (url.length() >= s.length())
                {
                    if (url.substr(0, s.length()).find(s, 0) != string::npos)
                    {
                        if (s.compare("4chan.org") == 0 || s.compare("4channel.org") == 0 ||
                            s.compare("boards.4chan.org") == 0 || s.compare("boards.4channel.org") == 0)
                        {
                            url = "a.4cdn.org" + url.substr(s.length(), url.length() - s.length());
                            url += ".json";
                        }

                        b_valid = true;
                        break;
                    }
                }
            }

            if (!b_valid)
            {
                return;
            }

            // split URL
            vector<string> url_parts = split(url, "/");

            // incomplete URL
            if (url_parts.size() < 2)
            {
                return;
            }

            // .json or some other file type
            if (url_parts[0].compare("a.4cdn.org") == 0)
            {
                website = ws_4chan;
            }
            // image or image thumbnail
            else if (url_parts[0].compare("i.4cdn.org") == 0)
            {
                if (url_parts.size() >= 3)
                {
                    vector<string> file_parts = split(url_parts[2], ".");
                    if (file_parts.size() == 2)
                    {
                        string fname = file_parts[0];
                        if (fname[fname.length() - 1] == 's')
                        {
                            pagetype = pt_image_thumbnail;
                        }
                        else
                        {
                            pagetype = pt_image;
                        }

                        board = url_parts[1];
                        file_name = url_parts[2];
                        file_ext = file_parts[1];
                        website = ws_4chan;

                        // valid url
                        return;
                    }
                }

                // invalid url
                return;
            }
            // flag or spoiler image
            else if (url_parts[0].compare("s.4cdn.org") == 0)
            {
                // flag
                if (url_parts.size() > 3)
                {
                    if (url_parts[2].compare("country") == 0)
                    {
                        file_name = url_parts[url_parts.size() - 1];
                        vector<string> file_parts = split(file_name, ".");
                        if (file_parts.size() == 2)
                        {
                            file_ext = file_parts[1];
                        }

                        pagetype = pt_image_flag;
                        website = ws_4chan;

                        // valid url
                        return;
                    }
                }
                // spoiler
                else if (url_parts.size() == 3)
                {
                    string s = "spoiler";
                    if (url_parts[2].substr(0, s.length()).find(s, 0) != string::npos)
                    {
                        file_name = url_parts[url_parts.size() - 1];
                        vector<string> file_parts = split(file_name, ".");
                        if (file_parts.size() == 2)
                        {
                            file_ext = file_parts[1];
                        }

                        pagetype = pt_image_spoiler;
                        website = ws_4chan;

                        // valid url
                        return;
                    }
                }

                // invalid url
                return;
            }

            // address = a.4cdn.org
            address = url_parts[0];
            board = url_parts[1];

            if (url_parts.size() == 4)
            {
                // thread
                // e.g. 'https://a.4cdn.org/po/thread/570368.json'
                if (url_parts[3].find(".json") != string::npos &&
                    url_parts[2].compare("thread") == 0)
                {
                    document_path = "/" + board + "/thread/" + url_parts[3];
                    thread_num_str = url_parts[3];
                    replace_substr(thread_num_str, ".json", "");
                    pagetype = e_page_type::pt_thread;
                }
            }
            else if (url_parts.size() == 3)
            {
                // catalog
                // e.g. 'https://a.4cdn.org/po/catalog.json'
                if (url_parts[2].compare("catalog.json") == 0)
                {
                    document_path = "/" + board + "/catalog.json";
                    pagetype = e_page_type::pt_board_catalog;
                }
                // board threads
                // e.g. 'https://a.4cdn.org/po/threads.json'
                else if (url_parts[2].compare("threads.json") == 0)
                {
                    document_path = "/" + board + "/threads.json";
                    pagetype = e_page_type::pt_board_threads;
                }
                // board archives
                // e.g. 'https://a.4cdn.org/po/archive.json'
                else if (url_parts[2].compare("archive.json") == 0)
                {
                    document_path = "/" + board + "/archive.json";
                    pagetype = e_page_type::pt_board_archive;
                }
                // board page (index)
                // e.g. 'https://a.4cdn.org/po/1.json'
                else if (url_parts[2].find(".json") != string::npos)
                {
                    vector<string> json_split = split(url_parts[2], ".");
                    if (json_split.size() < 2)
                    {
                        return;
                    }

                    string s = json_split[0];
                    if (!s.empty() && s.find_first_not_of("0123456789") == string::npos)
                    {
                        document_path = "/" + board + "/" + url_parts[2];
                        pagetype = e_page_type::pt_board_page;
                    }
                }
            }

            // boards list
            // e.g. 'https://a.4cdn.org/boards.json'
            if (url.find("a.4cdn.org/boards.json") != string::npos)
            {
                board = "";
                document_path = "/boards.json";
                pagetype = e_page_type::pt_boards_list;
            }

            // use https protocol
            url = "https://" + url;
        }

    };  // struct url_parser

};

