/**
 * Comfy
 *
 *  Copyright 2019 by Wolfish <wolfish@airmail.cc>
 *  https://wolfish.neocities.org/soft/comfy/
 *
 *  Licensed under the GPL v2.0 only.
 */
#pragma once

#include <iostream>
#include <sstream>
#include <cctype>
#include <algorithm>
#include <memory>
#include <map>
#include <vector>
#include <set>
#include <chrono>
#include <codecvt>
#include <locale> 
#include <unistd.h>

#include "../termbox/src/termbox.h"
#include "../gason/src/gason.h"

#include "stringutils.h"
#include "htmlutils.h"
#include "color.h"


#define WIDGET_MAN          WidgetMan::get_instance()
#define THREAD_MAN          ThreadMan::get_instance()
#define IMG_MAN             ImgMan::get_instance()


static const std::string VERSION = "v. 1.0.1";

extern std::string DATA_DIR;
extern std::string IMAGEBOARDS_DIR;
extern std::string TEMP_DIR;
extern std::string FLAGS_DIR;
static const std::string SAVE_FILE = ".comfy.save";

static const int POST_IMG_H = 20;   // in term cells
static const int FLAG_IMG_H = 1;    // in term cells

extern Colors::color_scheme COLO;
extern bool DISPLAY_IMAGES;


// Time

static std::chrono::milliseconds time_now_s()
{
    using namespace std::chrono;
    return duration_cast<seconds>(system_clock::now().time_since_epoch());
}


static std::chrono::milliseconds time_now_ms()
{
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch());
}


static std::chrono::microseconds time_now_us()
{
    using namespace std::chrono;
    return duration_cast<microseconds>(system_clock::now().time_since_epoch());
}


static std::chrono::nanoseconds time_now_ns()
{
    using namespace std::chrono;
    return duration_cast<nanoseconds>(system_clock::now().time_since_epoch());
}


// Dirs

static std::string get_file_save_dir(HTML_Utils::url_parser& parser, std::string thread_num_str = "")
{
    using namespace std;
    using namespace HTML_Utils;

    std::string path = IMAGEBOARDS_DIR;

    switch(parser.website)
    {
        case ws_4chan:
            path += "4chan/";
            break;

        // unknown url, save in temp dir
        case ws_UNKNOWN:
            return TEMP_DIR;

        default:
            return TEMP_DIR;
    }

    switch(parser.pagetype)
    {
        case pt_UNKNOWN:
            return TEMP_DIR;

        // TODO: put these files in a proper dir
        case pt_boards_list:
            return path;

        case pt_board_page:
            return TEMP_DIR;

        case pt_board_catalog:
            path += parser.board + "/";
            return path;

        case pt_board_threads:
            return TEMP_DIR;

        case pt_board_archive:
            return TEMP_DIR;

        case pt_thread:
            path += parser.board + "/threads/" + parser.thread_num_str + "/";
            return path;

        case pt_image:
            // don't know where to put image if thread number not provided
            if (thread_num_str.empty())
            {
                return TEMP_DIR;
            }
            else
            {
                path += parser.board + "/threads/" + thread_num_str + "/";
                return path;
            }

        case pt_image_thumbnail:
            // don't know where to put image if thread number not provided
            if (thread_num_str.empty())
            {
                return TEMP_DIR;
            }
            else
            {
                path += parser.board + "/threads/" + thread_num_str + "/";
                return path;
            }

        case pt_image_flag:
            path += "flags/";
            return path;

        case pt_image_spoiler:
            path += parser.board + "/spoilers/";
            return path;

        default:
            return TEMP_DIR;
    }
}

// returns the save directory path for a file retrieved from param url
// path returned includes a trailing slash
static std::string get_file_save_dir(std::string url, std::string thread_num_str = "")
{
    HTML_Utils::url_parser parser(url);
    return get_file_save_dir(parser, thread_num_str);
}


// Imageboard

namespace imageboard
{
    using namespace std;

    struct post
    {
        post()
        : b_op(false)
        , b_sticky(false)
        , b_closed(false)
        , b_archived(false)
        , b_bumplimit(false)
        , b_imagelimit(false)
        , replies(0)
        , images(0)
        , unique_ips(0)
        
        , num(0)
        , thread_num(0)
        , subject("")
        , datetime("")

        , name("")
        , trip("")
        , id("")
        , capcode("")
        , text("")

        , img_filename("")
        , img_ext("")
        , img_w(0)
        , img_h(0)
        , img_thumb_w(0)
        , img_thumb_h(0)
        , img_time(0)
        , img_md5("")
        , img_fsize(0)

        , b_img_deleted(false)
        , b_img_spoilered(false)
        , b_custom_spoiler(false)

        , country("")
        , troll_country("")
        , country_name("")
        {}


        bool b_op;
        bool b_sticky;
        bool b_closed;
        bool b_archived;
        bool b_bumplimit;
        bool b_imagelimit;
        int replies;
        int images;
        int unique_ips;

        int num;
        int thread_num;     // post num of thread OP
        string subject;
        string datetime;

        string name;
        string trip;
        string id;
        string capcode;
        string text;

        string img_filename;
        string img_ext;
        int img_w;
        int img_h;
        int img_thumb_w;
        int img_thumb_h;
        int64_t img_time;
        string img_md5;
        int img_fsize;

        bool b_img_deleted;
        bool b_img_spoilered;
        bool b_custom_spoiler;

        string country;         // e.g. CA
        string troll_country;   // e.g. CA ;^)
        string country_name;    // e.g. Canada
    };


    struct board_listing
    {
        string path;
        string title;
    };


    struct page_data
    {
        string url;
        string board;
        string thread_num_str;      // post num of thread OP
        vector<post> posts;
        vector<board_listing> board_listings;
    };
};


// JSON

namespace JSON_Utils
{
    using namespace std;
    using namespace imageboard;

    struct json
    {
        JsonValue value;
        JsonAllocator allocator;
    };


    static post parse_4chan_post(JsonValue* j)
    {
        post p;
        if (!j) return p;

        // post data
        for (auto k : *j)
        {
            string key = k->key;

            if (key.compare("no") == 0)
            {
                p.num = k->value.toNumber();
            }
            else if (key.compare("resto") == 0)
            {
                p.thread_num = k->value.toNumber();
                if (p.thread_num == 0)
                {
                    p.b_op = true;
                }
            }
            else if (key.compare("sticky") == 0)
            {
                p.b_sticky = true;
            }
            else if (key.compare("closed") == 0)
            {
                p.b_closed = true;
            }
            else if (key.compare("now") == 0)
            {
                p.datetime = k->value.toString();
            }
            else if (key.compare("name") == 0)
            {
                p.name = k->value.toString();
            }
            else if (key.compare("trip") == 0)
            {
                p.trip = k->value.toString();
            }
            else if (key.compare("id") == 0)
            {
                p.id = k->value.toString();
            }
            else if (key.compare("capcode") == 0)
            {
                p.capcode = k->value.toString();
            }
            else if (key.compare("country") == 0)
            {
                p.country = k->value.toString();
            }
            else if (key.compare("troll_country") == 0)
            {
                p.troll_country = k->value.toString();
            }
            else if (key.compare("country_name") == 0)
            {
                p.country_name = k->value.toString();
            }
            else if (key.compare("sub") == 0)
            {
                p.subject = k->value.toString();
            }
            else if (key.compare("com") == 0)
            {
                p.text = k->value.toString();
            }
            else if (key.compare("tim") == 0)
            {
                p.img_time = k->value.toNumber();
            }
            else if (key.compare("filename") == 0)
            {
                p.img_filename = k->value.toString();
            }
            else if (key.compare("ext") == 0)
            {
                p.img_ext = k->value.toString();
            }
            else if (key.compare("fsize") == 0)
            {
                p.img_fsize = k->value.toNumber();
            }
            else if (key.compare("md5") == 0)
            {
                p.img_md5 = k->value.toString();
            }
            else if (key.compare("w") == 0)
            {
                p.img_w = k->value.toNumber();
            }
            else if (key.compare("h") == 0)
            {
                p.img_h = k->value.toNumber();
            }
            else if (key.compare("tn_w") == 0)
            {
                p.img_thumb_w = k->value.toNumber();
            }
            else if (key.compare("tn_h") == 0)
            {
                p.img_thumb_h = k->value.toNumber();
            }
            else if (key.compare("filedeleted") == 0)
            {
                p.b_img_deleted = true;
            }
            else if (key.compare("spoiler") == 0)
            {
                p.b_img_spoilered = true;
            }
            else if (key.compare("custom_spoiler") == 0)
            {
                p.b_custom_spoiler = true;
            }
            else if (key.compare("replies") == 0)
            {
                p.replies = k->value.toNumber();
            }
            else if (key.compare("images") == 0)
            {
                p.images = k->value.toNumber();
            }
            else if (key.compare("bumplimit") == 0)
            {
                p.b_bumplimit = true;
            }
            else if (key.compare("imagelimit") == 0)
            {
                p.b_imagelimit = true;
            }
            else if (key.compare("unique_ips") == 0)
            {
                p.unique_ips = k->value.toNumber();
            }
            else if (key.compare("archived") == 0)
            {
                p.b_archived = true;
            }
        }

        return p;
    }


    static bool parse_4chan_boards_list(const char* json_data, page_data& data)
    {
        json json_dom;
        char *endptr;
        int status = jsonParse(json_data, &endptr, &(json_dom.value), json_dom.allocator);
        if (status != JSON_OK)
        {
            return false;
        }

        if (json_dom.value.getTag() == JSON_OBJECT)
        {
            for (auto i : json_dom.value)
            {
                if (i->value.getTag() == JSON_ARRAY)
                {
                    for (auto j : i->value)
                    {
                        if (j->value.getTag() == JSON_OBJECT)
                        {
                            board_listing b;

                            for (auto k : j->value)
                            {
                                string key = k->key;

                                if (key.compare("board") == 0)
                                {
                                    b.path = k->value.toString();
                                }
                                if (key.compare("title") == 0)
                                {
                                    b.title = k->value.toString();
                                }
                            }

                            data.board_listings.push_back(b);
                        }
                    }
                }
            }
        }

        return true;
    }


    static bool parse_4chan_thread(const char* json_data, page_data& data)
    {
        json json_dom;
        char *endptr;
        int status = jsonParse(json_data, &endptr, &(json_dom.value), json_dom.allocator);
        if (status != JSON_OK)
        {
            return false;
        }

        if (json_dom.value.getTag() == JSON_OBJECT)
        {
            for (auto i : json_dom.value)
            {
                string key = i->key;

                // posts array
                if (key.compare("posts") == 0)
                {
                    if (i->value.getTag() == JSON_ARRAY)
                    {
                        for (auto j : i->value)
                        {
                            // post
                            if (j->value.getTag() == JSON_OBJECT)
                            {
                                post p = parse_4chan_post(&j->value);
                                data.posts.push_back(p);
                            }
                        }
                    }
                }
            }
        }

        return true;
    }


    static bool parse_4chan_catalog(const char* json_data, page_data& data)
    {
        json json_dom;
        char *endptr;
        int status = jsonParse(json_data, &endptr, &(json_dom.value), json_dom.allocator);
        if (status != JSON_OK)
        {
            return false;
        }

        if (json_dom.value.getTag() == JSON_ARRAY)
        {
            // catalog pages
            for (auto i : json_dom.value)
            {
                if (i->value.getTag() == JSON_OBJECT)
                {
                    // items in page object
                    for (auto j : i->value)
                    {
                        string key = j->key;
                        // threads array (they are posts)
                        if (key.compare("threads") == 0)
                        {
                            for (auto k : j->value)
                            {
                                post p = parse_4chan_post(&k->value);
                                data.posts.push_back(p);
                            }
                        }
                    }
                }
            }
        }

        return true;
    }

};


// Enums

enum e_widget_sizing
{
    // stretches to fill terminal screen
    ws_fullscreen,
    // stretches widget to fill parent widget
    ws_fill,
    // stretches to fill parent space, but
    // size is manually managed by the parent
    // (e.g. a horizontal box dividing up
    // horizontal space to fill widgets evenly)
    ws_fill_managed,
    // sizes widget to size of child widget
    ws_auto,
    // keeps the widget a fixed size
    ws_fixed,
    // widget dynamically resizes itself
    ws_dynamic,
};


enum e_widget_align
{
    wa_left,
    wa_center,
    wa_right,
    wa_top,
    wa_bottom,
};


// Data Structures

struct vector2d
{
    vector2d()
    : x(0), y(0)
    {}

    vector2d(int i)
    : x(i), y(i)
    {}

    vector2d(int _x, int _y)
    : x(_x), y(_y)
    {}


    int x;
    int y;


    bool is_zero() const
    {
        return x == 0 && y == 0;
    }

    inline void operator=(const vector2d& other)
    {
        x = other.x;
        y = other.y;
    }

    inline void operator+=(const vector2d& other)
    {
        x += other.x;
        y += other.y;
    }

    inline void operator-=(const vector2d& other)
    {
        x -= other.x;
        y -= other.y;
    }

    inline vector2d operator+(const vector2d& other)
    {
        return vector2d(x + other.x, y + other.y);
    }

    inline vector2d operator-(const vector2d& other)
    {
        return vector2d(x - other.x, y - other.y);
    }

    inline bool operator==(const vector2d& other)
    {
        return x == other.x && y == other.y;
    }

    inline bool operator!=(const vector2d& other)
    {
        return x != other.x || y != other.y;
    }
};


struct vector4d
{
    vector4d()
    : a(0), b(0), c(0), d(0)
    {}

    vector4d(int x)
    : a(x), b(x), c(x), d(x)
    {}

    vector4d(int _a, int _b, int _c, int _d)
    : a(_a), b(_b), c(_c), d(_d)
    {}


    int a;
    int b;
    int c;
    int d;


    bool is_zero() const
    {
        return a == 0 && b == 0 && c == 0 && d == 0;
    }

    inline vector4d operator+(const vector4d& other)
    {
        return vector4d(a + other.a, b + other.b, c + other.c, d + other.d);
    }

    inline void operator+=(const vector4d& other)
    {
        a += other.a;
        b += other.b;
        c += other.c;
        d += other.d;
    }

    inline void operator-=(const vector4d& other)
    {
        a -= other.a;
        b -= other.b;
        c -= other.c;
        d -= other.d;
    }

    inline void operator/=(const int& other)
    {
        a = (float)a / (float)other;
        b = (float)b / (float)other;
        c = (float)c / (float)other;
        d = (float)d / (float)other;
    }

    inline bool operator==(const vector4d& other)
    {
        return a == other.a && b == other.b && c == other.c && d == other.d;
    }

    inline bool operator!=(const vector4d& other)
    {
        return a != other.a || b != other.b || c != other.c || d != other.d;
    }
};


struct term_cell
{
    term_cell()
    : coord(vector2d())
    {
        tb_data.ch = TB_NULL_CHAR;
        tb_data.bg = 0;
        tb_data.fg = 0;
    }

    term_cell(
        vector2d _coord)
    : coord(_coord)
    {
        tb_data.ch = TB_NULL_CHAR;
        tb_data.bg = 0;
        tb_data.fg = 0;
    }

    term_cell(
        tb_cell _tb_data,
        vector2d _coord
    )
    : tb_data(_tb_data)
    , coord(_coord)
    {}


    tb_cell tb_data;
    vector2d coord;


    inline void operator=(const term_cell& other)
    {
        coord = other.coord;
        tb_data.ch = other.tb_data.ch;
        tb_data.bg = other.tb_data.bg;
        tb_data.fg = other.tb_data.fg;
    }

    bool is_null() const
    {
        return tb_data.ch == TB_NULL_CHAR;
    }
};


struct term_word
{
    term_word()
    : text(std::wstring())
    , bg(0)
    , fg(15)
    , b_bold(false)
    , b_underline(false)
    , b_reverse(false)
    , b_newline(false)
    {}

    term_word(
        std::wstring _text,
        uint32_t _bg,
        uint32_t _fg,
        bool _b_bold = false,
        bool _b_underline = false,
        bool _b_reverse = false
    )
    : text(_text)
    , bg(_bg)
    , fg(_fg)
    , b_bold(_b_bold)
    , b_underline(_b_underline)
    , b_reverse(_b_reverse)
    , b_newline(false)
    {}

    term_word(
        wchar_t* _text,
        uint32_t _bg,
        uint32_t _fg,
        bool _b_bold = false,
        bool _b_underline = false,
        bool _b_reverse = false
    )
    : text(_text)
    , bg(_bg)
    , fg(_fg)
    , b_bold(_b_bold)
    , b_underline(_b_underline)
    , b_reverse(_b_reverse)
    , b_newline(false)
    {}

    std::wstring text;
    uint32_t bg;
    uint32_t fg;
    bool b_bold;
    bool b_underline;
    bool b_reverse;
    bool b_newline;

    static term_word newline()
    {
        term_word word;
        word.b_newline = true;
        return word;
    }

    inline void operator=(const term_word& other)
    {
        text = other.text;
        bg = other.bg;
        fg = other.fg;
        b_bold = other.b_bold;
        b_underline = other.b_underline;
        b_reverse = other.b_reverse;
        b_newline = other.b_newline;
    }
};


struct user_input
{
    user_input()
    : ch(-1)
    , input_event(tb_event())
    {}

    uint32_t ch;
    tb_event input_event;
};


// Error output

static inline void ERR(std::string out, std::string label = "")
{
    if (!label.empty()) label += ": ";
    std::cerr << label << out << std::endl;
}


static inline void ERR(std::wstring out, std::string label = "")
{
    if (!label.empty()) label += ": ";
    std::cerr << label << wstr_to_string(out) << std::endl;
}


static inline void ERR(const char* out, std::string label = "")
{
    if (!label.empty()) label += ": ";
    std::cerr << label << out << std::endl;
}


static inline void ERR(vector2d out, std::string label = "")
{
    if (!label.empty()) label += ": ";
    std::cerr << label << out.x << "  x  " << out.y << std::endl;
}


static inline void ERR(vector4d out, std::string label = "")
{
    if (!label.empty()) label += ": ";
    std::cerr << label << "a: " << out.a << " b: " << out.b << " c: " << out.c << " d: " << out.d << std::endl;
}


static inline void ERR(int out, std::string label = "")
{
    if (!label.empty()) label += ": ";
    std::cerr << label << out << std::endl;
}


static inline void ERR(long out, std::string label = "")
{
    if (!label.empty()) label += ": ";
    std::cerr << label << out << std::endl;
}


static inline void ERR(float out, std::string label = "")
{
    if (!label.empty()) label += ": ";
    std::cerr << label << out << std::endl;
}


static inline void ERR(size_t out, std::string label = "")
{
    if (!label.empty()) label += ": ";
    std::cerr << label << out << std::endl;
}


// Benchmarking

namespace Benchmark
{
    using namespace std::chrono;

    static nanoseconds time_ns;
    static milliseconds time_ms;

    static void start_ns()
    {
        time_ns = time_now_ns();
    }

    static void end_ns(std::string label = "Benchmark: ")
    {
        nanoseconds delta = time_now_ns() - time_ns;
        long time = delta.count();
        ERR(time, label);
    }


    static void start_ms()
    {
        time_ms = time_now_ms();
    }

    static void end_ms(std::string label = "Benchmark: ")
    {
        milliseconds delta = time_now_ms() - time_ms;
        long time = delta.count();
        ERR(time, label);
    }
};

