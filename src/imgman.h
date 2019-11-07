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

// X11
#include <X11/Xlib.h>

/**
 *   w3mimgdisplay's code was studied and adapted to
 *   figure out how to draw images to the term window
 *   using Imlib2 and X11.
 */


struct x11_info
{
    x11_info()
    : display(nullptr)
    , window(-1)
    , parent(-1)
    , visual(nullptr)
    , depth(0)
    , background_pixel(0)
    , imageGC(nullptr)
    , offset_x(0)
    , offset_y(0)
    , b_init(false)
    {}

    ~x11_info()
    {
        if (imageGC)
        {
            XFreeGC(display, imageGC);
        }
    }


    Display* display;
    Window window, parent;
    Visual* visual;
    int depth;
    unsigned long background_pixel;
    GC imageGC;
    int offset_x;
    int offset_y;
    bool b_init;


    vector2d get_win_size()
    {
        if (!b_init || !display) return vector2d();

        vector2d size;
        XWindowAttributes attr;
        XGetWindowAttributes(display, window, &attr);
        size.x = attr.width;
        size.y = attr.height;

        return size;
    }


    void free_pixmap(Pixmap pixmap)
    {
        if (pixmap != -1 && display)
        {
            XFreePixmap(display, pixmap);
            pixmap = -1;
            //ERR("FREEING PIXMAP IN __XI__");
        }
    }


    void init()
    {
        if (!DISPLAY_IMAGES || b_init) return;

        // these are initialized to 2 for some reason
        offset_x = 2;
        offset_y = 2;

        char* id;
        int revert, i;
        unsigned int nchildren;
        XWindowAttributes attr;
        Window root, *children;

        display = XOpenDisplay(nullptr);
        // error
        if (display == nullptr) return;

        if ((id = getenv("WINDOWID")) != NULL)
        {
            window = (Window) atoi(id);
        }
        else
        {
            XGetInputFocus(display, &window, &revert);
        }

        // error
        if (!window) return;

        // window size
        XGetWindowAttributes(display, window, &attr);
        int win_w = attr.width;
        int win_h = attr.height;
        visual = attr.visual;
        depth = attr.depth;

        while (1)
        {
            Window p_window;

            XQueryTree(display, window, &root, &parent,
                   &children, &nchildren);
            p_window = window;

            for (i = 0; i < nchildren; i++)
            {
                XGetWindowAttributes(display, children[i], &attr);

                if (attr.width > win_w * 0.7 &&
                attr.height > win_h * 0.7)
                {
                    /* maybe text window */
                    win_w = attr.width;
                    win_h = attr.height;
                    visual = attr.visual;
                    depth = attr.depth;
                    window = children[i];
                }
            }

            if (p_window == window) break;
        }

        for (i = 0; i < nchildren; i++)
        {
            XGetWindowAttributes(display, children[i], &attr);

            if (attr.x <= 0 && attr.width < 30 &&
                attr.height > win_h * 0.7)
            {
                /* scrollbar of xterm/kterm ? */
                offset_x += attr.x + attr.width + attr.border_width * 2;
                break;
            }
        }

        if (!imageGC)
        {
            imageGC = XCreateGC(display, parent, 0, NULL);
            // error
            if (!imageGC) return;
        }

        b_init = true;
    }


    bool is_valid() const
    {
        return b_init;
    }

};


struct img_packet
{
    img_packet()
    : size(vector2d())
    , widget_id("")
    , post_key(-1)
    , parser(HTML_Utils::url_parser())
    , image_key("")
    {}

    img_packet(
        vector2d _size,
        std::string _widget_id,
        int _post_key,
        HTML_Utils::url_parser _parser,
        std::string _file_path
    )
    : size(_size)
    , widget_id(_widget_id)
    , post_key(_post_key)
    , parser(_parser)
    , file_path(_file_path)
    , image_key("")
    {}


    // size in term cells
    vector2d size;
    // destination widget
    std::string widget_id;
    // post number/key
    int post_key;
    // url data
    HTML_Utils::url_parser parser;
    // file path to dir of image on disk (excluding file name)
    std::string file_path;
    // image key in cache map
    std::string image_key;
    // if the packet goes unclaimed,
    // the cache is garbage collected
    // when this pointer is destroyed
    // (if it's the last shared_ptr for this token)
    std::shared_ptr<checkout_token> img_token;


    bool is_video() const
    {
        return parser.file_ext.compare("mp4") == 0 ||
            parser.file_ext.compare("webm") == 0 ||
            parser.file_ext.compare("mpeg") == 0 ||
            parser.file_ext.compare("mpg") == 0 ||
            parser.file_ext.compare("MP4") == 0 ||
            parser.file_ext.compare("WEBM") == 0 ||
            parser.file_ext.compare("MPEG") == 0 ||
            parser.file_ext.compare("MPG") == 0;
    }
};


class ImgMan
{
public:

    struct img_data
    {
        public:

        img_data()
        : image_path("")
        , size(vector2d())
        , pixmap(-1)
        , width(0)
        , height(0)
        {}

        ~img_data();


        std::string image_path;
        // actual size of image
        vector2d size;

        //vips::VImage vimg;

        private:
        // set access to this var to private so that
        // we can force the old pixmap to be garbage
        // collected when a new one is set
        Pixmap pixmap;

        public:
        // width and height of pixmap
        int width;
        int height;


        void set_pixmap(Pixmap _pixmap);
        Pixmap get_pixmap() const { return pixmap; };
        void invalidate_pixmap();

        bool valid_pixmap() const
        {
            return pixmap != -1;
        }

        inline void operator=(const img_data& other)
        {
            // move the vimg, since it could have a large size
            //vimg = std::move(other.vimg);
            image_path = other.image_path;
            size = other.size;
            width = other.width;
            height = other.height;

            // transfer ownership of pixmap to new copy
            // so that the pixmap is not freed in the
            // destructor of other
            set_pixmap(other.pixmap);
            const_cast<img_data&>(other).pixmap = -1;
        }

    };


    struct sixel_draw
    {
        sixel_draw()
        : image_data(nullptr)
        , offset(vector2d())
        , size(vector2d())
        , coord(vector2d())
        {}

        sixel_draw(
            img_data* _image_data,
            vector2d _offset,
            vector2d _size,
            vector2d _coord
        )
        : image_data(_image_data)
        , offset(_offset)
        , size(_size)
        , coord(_coord)
        {}


        img_data* image_data;
        vector2d offset;
        vector2d size;
        vector2d coord;
    };


    // leave the constructor empty so that 
    // nothing is executed when the static
    // instance is fetched
    ImgMan() {};

    // delete these functions for use as singleton
    ImgMan(ImgMan const&)             = delete;
    void operator=(ImgMan const&)    = delete;

    static ImgMan& get_instance()
    {
        static ImgMan img_man;
        return img_man;
    }

    init();
    void shutdown();

    // x11 info
    x11_info xi;

    void request_image(std::string url, vector2d size, std::string widget_id, std::string thread_num_str = "", int post_num = -1);

    void free_pixmap(Pixmap pixmap);
    /*
    *  x  - x coordinate to draw the image at (top left corner)
    *  y  - y coordinate to draw the image at (top left corner)
    *  w  - width to draw the image
    *  h  - height to draw the image
    *  sx - x offset to draw the image
    *  xy - y offset to draw the image
    *  sw - width of the original (source) image
    *  sh - height of the original (source) image
    */
    bool show_image(img_data& img, int sx, int sy, int sw, int sh, int x, int y);
    bool load_pixmap(img_data& img, const int w, const int h);

    // multithreading
    std::shared_mutex imlib_mtx;
    // maps image path on disk to image data
    threadsafe_map<std::string, img_data> img_cache_map;
    // get a checkout_token to ensure an image is not deleted
    // from the cache as long as at least one token is 
    // checked out for that image
    std::shared_ptr<checkout_token> checkout_img(std::string key);
    // pass to ThreadMan
    // widget_id is the widget the image is to be directed to
    static void threaded_load_img_from_disk(img_packet pac);
    void load_img_from_disk(img_packet pac, std::string job_pool_id = DEFAULT_JOB_POOL_ID);
    // img_packet queue
    threadsafe_queue<img_packet> queue__image_packet;


    vector2d char_size = vector2d(-1);
    std::vector<sixel_draw> sixel_draw_buffer;

    vector2d get_term_size_in_pixels();
    vector2d get_term_size_in_chars();
    // the first call to this function calculates
    // the term character/cell size in pixels
    // based on the terminal window size, the stores it
    // in a variable. the window size used in the calculation
    // does not change after a window resize; the window size
    // is only correct between the time the program first starts
    // and the window is resized for the first time (if at at all)
    vector2d get_term_char_size();

    // load the image from disk and caches its size
    // if it is not already present in the cache.
    // returns the image's key in the cache map.
    // expects w and h in pixels
    std::string cache_img(std::string path, int w, int h);

    vector2d get_img_size(std::string path);
    vector2d get_img_size_in_term_chars(std::string path);

    // clears images in a rectangle, units are term char size
    // x_0 and y_0 = upper left corner
    // x_1 and y_1 = bottom right corner
    void clear_term_cells(int x_0, int y_0, int x_1, int y_1);

    void wipe_screen();
    void sync();
    void redraw_buffer(bool b_sync = true);
    void clear_buffer() { sixel_draw_buffer.clear(); };

    // actual size is the actual image size
    // image width or height of 0 = use actual image size
    // image width or height < 0 = maintain aspect ratio relative to value > -1
    // returns the dimensions of the image drawn in pixels
    vector2d calc_img_size_pixels(vector2d img_size, int width = 0, int height = 0);
    // actual size is the actual image size
    // image width or height of 0 = use actual image size
    // image width or height < 0 = maintain aspect ratio relative to value > -1
    // returns the dimensions of the image drawn in term cells (term char size)
    vector2d calc_img_size_term_chars(vector2d img_size, int width = 0, int height = 0);

    // all dimensions and coordinates are in pixels
    // image width or height of 0 = use actual image size
    // image width or height < 0 = maintain aspect ratio relative to value > -1
    // crop: a = left, b = top, c = right, d = bottom
    // returns the dimensions of the image drawn in pixels
    vector2d draw_img_pixel_coord(
        img_data& image_data,
        std::string path,
        int width = 0,
        int height = 0,
        int coord_x = 0,
        int coord_y = 0,
        vector4d crop = vector4d()
    );

    // all variables are in terminal char units
    // image width or height of 0 = use actual image size
    // image width or height < 0 = maintain aspect ratio relative to value > -1
    // crop: a = left, b = top, c = right, d = bottom
    // returns the dimensions of the image drawn in term cells (term char size)
    vector2d draw_img_term_coord_and_size(
        img_data& image_data,
        std::string path,
        int width = 0,
        int height = 0,
        int coord_x = 0,
        int coord_y = 0,
        vector4d crop = vector4d()
    );

    // all variables are in terminal char units
    // image width or height of 0 = use actual image size
    // image width or height < 0 = maintain aspect ratio relative to value > -1
    // crop: a = left, b = top, c = right, d = bottom
    // returns the dimensions of the image drawn in term cells (term char size)
    vector2d draw_img_term_coord_and_size(
        std::string path,
        int width = 0,
        int height = 0,
        int coord_x = 0,
        int coord_y = 0,
        vector4d crop = vector4d()
    );

};

