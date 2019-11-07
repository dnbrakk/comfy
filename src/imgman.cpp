/**
 * Comfy
 *
 *  Copyright 2019 by Wolfish <wolfish@airmail.cc>
 *  https://wolfish.neocities.org/soft/comfy/
 *
 *  Licensed under the GPL v2.0 only.
 */
#include "imgman.h"
#include "fileops.h"
#include "netops.h"
#include "widgetman.h"
#include <complex>

// X11
#include <X11/Xutil.h>

// imlib2
#include <Imlib2.h>


static int x11_err(Display *, XErrorEvent *)
{
    DISPLAY_IMAGES = false;
    ERR("X11 ERROR");
    return 0;
}


ImgMan::init()
{
    if (DISPLAY_IMAGES)
    {
        // init X11 multithreading support
        //
        // the following can be used to lock/unlock
        // the X display (basically mutexes):
        //
        //      void XLockDisplay(Display *display)
        //      void XUnlockDisplay(Display *display)
        //
        int init = XInitThreads();
        if (init == 0)
        {
            ERR("X11 multithreading failed to initialize.");
        }

        // catch errors so program won't halt on error
        XSetErrorHandler(&x11_err);

        // x11 info
        xi.init();

        // cache term cell/char size (in pixels)
        // note:    must be called after xi.init()
        get_term_char_size();

        // imlib
        std::unique_lock<std::shared_mutex> lck(imlib_mtx);
        imlib_set_cache_size(0);            // no image cache
        imlib_set_font_cache_size(0);
    }
}


void ImgMan::shutdown()
{
    if (!DISPLAY_IMAGES) return;

    clear_buffer();

    for (const auto& img_d : img_cache_map.map)
    {
        free_pixmap(img_d.second.get_pixmap());
    }
}


void ImgMan::request_image(std::string url, vector2d size, std::string widget_id, std::string thread_num_str, int post_num)
{
    http_image_req req(
        url,
        size,
        widget_id,
        thread_num_str,
        post_num
    );

    // load from disk
    if (FileOps::file_exists(req.get_file_path() + req.get_file_name()))
    {
        img_packet pac(
            size,
            widget_id,
            post_num,
            req.parser,
            req.get_file_path());

        IMG_MAN.load_img_from_disk(pac, widget_id /* job_pool_id */);
    }
    // create multithreaded http get request
    else
    {
        NetOps::http_get__image(req, widget_id /* job_pool_id */);
    }
}

std::shared_ptr<checkout_token> ImgMan::checkout_img(std::string key)
{
    return img_cache_map.checkout(key);
}


void ImgMan::img_data::set_pixmap(Pixmap _pixmap)
{
    if (valid_pixmap())
    {
        IMG_MAN.free_pixmap(pixmap);
    }

    pixmap = _pixmap;
}


void ImgMan::img_data::invalidate_pixmap()
{
    set_pixmap(-1);
}


ImgMan::img_data::~img_data()
{
    std::vector<sixel_draw>& buf = IMG_MAN.sixel_draw_buffer;
    // remove image_data from buffer, if necessary
    for (int i = buf.size() - 1; i >= 0; --i)
    {
        sixel_draw& draw = buf[i];
        if (this == draw.image_data)
        {
            buf.erase(buf.begin() + i);
        }
    }

    IMG_MAN.free_pixmap(pixmap);
}


void ImgMan::free_pixmap(Pixmap pixmap)
{
    if (!DISPLAY_IMAGES) return;

    xi.free_pixmap(pixmap);
}


// returns the key of the image data in the cache map
std::string ImgMan::cache_img(std::string path, int w, int h)
{
    if (!DISPLAY_IMAGES) return "";

    std::string key = path;
    // add dimensions to key
    key += "_" + std::to_string(w) + "x" + std::to_string(h);

    img_data* data = img_cache_map.get(key);
    // already exists in cache, return key
    if (data)
    {
        return key;
    }
    // load and cache image
    else
    {
        img_data img;
        img.image_path = path;

        {
            // MUTEX
            std::unique_lock<std::shared_mutex> lck(imlib_mtx);

            Imlib_Image im = imlib_load_image_without_cache(path.c_str());
            if (im)
            {
                imlib_context_set_image(im);
                img.size.x = imlib_image_get_width();
                img.size.y = imlib_image_get_height();

                vector2d img_size = calc_img_size_pixels(img.size, w, h);
                w = img_size.x;
                h = img_size.y;

                img.set_pixmap(
                    XCreatePixmap(xi.display, xi.parent, w, h, xi.depth));
                // error
                if (!img.valid_pixmap()) 
                {
                    imlib_free_image();
                    return "";
                }

                XSetForeground(xi.display, xi.imageGC, xi.background_pixel);
                XFillRectangle(xi.display, img.get_pixmap(), xi.imageGC, 0, 0, w, h);

                // write to pixmap
                imlib_context_set_display(xi.display);
                imlib_context_set_visual(DefaultVisual(xi.display, 0));
                imlib_context_set_colormap(DefaultColormap(xi.display, 0));
                imlib_context_set_drawable((Drawable) img.get_pixmap());
                imlib_render_image_on_drawable_at_size(0, 0, w, h);

                imlib_free_image();

                img.width = w;
                img.height = h;
            }

            // imlib MUTEX destructs
        }

        // make sure to modify map outside of imlib mutex
        // so as to prevent locking multiple mutexes simultaneously
        if (img.valid_pixmap())
        {
            // add to cache map
            img_cache_map.insert(key, img);

            return key;
        }
    }

    // nothing added to cache, return empty key
    return "";
}


void ImgMan::threaded_load_img_from_disk(img_packet pac)
{
    if (!DISPLAY_IMAGES) return;

    int w = 1;
    int h = 1;
    vector2d char_size = IMG_MAN.get_term_char_size();
    w = pac.size.x * char_size.x;
    h = pac.size.y * char_size.y;

    pac.image_key = IMG_MAN.cache_img(
        pac.file_path + pac.parser.file_name, w, h);
    // ensure image is removed from cache if packet is not claimed
    pac.img_token = IMG_MAN.checkout_img(pac.image_key);

    IMG_MAN.queue__image_packet.push(pac);
}


void ImgMan::load_img_from_disk(img_packet pac, std::string job_pool_id)
{
    if (!DISPLAY_IMAGES) return;

    THREAD_MAN.enqueue_job(
        std::bind(threaded_load_img_from_disk, pac), job_pool_id);
}


vector2d ImgMan::get_term_size_in_pixels()
{
    return xi.get_win_size();
}


vector2d ImgMan::get_term_size_in_chars()
{
    return WIDGET_MAN.get_term_size();
    //struct winsize w;
    //ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    //return vector2d(w.ws_row, w.ws_col);
}


vector2d ImgMan::get_term_char_size()
{
    // only calculate char size once
    if (char_size.x == -1 || char_size.y == -1)
    {
        vector2d pix = get_term_size_in_pixels();
        vector2d chars = get_term_size_in_chars();

        // error
        if (chars.x < 1 || chars.y < 1) return vector2d();

        char_size = vector2d(pix.x / chars.x, pix.y / chars.y);
    }

    return char_size;
}


vector2d ImgMan::get_img_size_in_term_chars(std::string path)
{
    vector2d img_size = get_img_size(path);
    vector2d char_size = get_term_char_size();
    return vector2d((float)img_size.x / (float)char_size.x,
                    (float)img_size.y / (float)char_size.y);
}


vector2d ImgMan::get_img_size(std::string path)
{
    vector2d size(-1);

    if (path.empty()) return size;

    img_data* img = img_cache_map.get(path);
    if (img)
    {
        size = img->size;
    }
    // load from disk
    else
    {
        // MUTEX
        std::unique_lock<std::shared_mutex> lck(IMG_MAN.imlib_mtx);

        Imlib_Image im = imlib_load_image_without_cache(path.c_str());
        if (im)
        {
            imlib_context_set_image(im);
            size.x = imlib_image_get_width();
            size.y = imlib_image_get_height();
            imlib_free_image();
        }
    }

    return size;
}


vector2d ImgMan::calc_img_size_pixels(vector2d img_size, int width, int height)
{
    if (img_size.x < 1 || img_size.y < 1) return vector2d();

    // if both height and width flagged as maintain aspect ratio,
    // use full image size
    if (width < 0 && height < 0)
    {
        return img_size;
    }

    // use actual image size
    if (width == 0) width = img_size.x;
    if (height == 0) height = img_size.y;

    // aspect ratio
    float ar = (float)img_size.x / (float)img_size.y;

    // width or height == -1
    // maintain aspect ratio (slave to given dimension)
    if (width < 0 || height < 0)
    {
        if (width < 0)
        {
            width = (float)height * ar;
            // scale image to fit aspect ratio of term cells
            // NOTE: this assumes height is already properly scaled
            vector2d char_size = get_term_char_size();
            width -= width % char_size.x;
            if (width < char_size.x) width = char_size.x;
        }
        else if (height < 0)
        {
            height = (float)width / ar;
            // scale image to fit aspect ratio of term cells
            // NOTE: this assumes width is already properly scaled
            vector2d char_size = get_term_char_size();
            height -= height % char_size.y;
            if (height < char_size.y) height = char_size.y;
        }
    }
    // both width and height > 0
    // maintain aspect ratio (slave to squished dimension)
    else
    {
        float ar_in = (float)width / (float)height;
        if (ar_in != ar)
        {
            // height is squished
            if (ar_in > ar)
            {
                width = (float)height * ar;
                // scale image to fit aspect ratio of term cells
                // NOTE: this assumes height is already properly scaled
                vector2d char_size = get_term_char_size();
                width -= width % char_size.x;
                if (width < char_size.x) width = char_size.x;
            }
            // width is squished
            else
            {
                height = (float)width / ar;
                // scale image to fit aspect ratio of term cells
                // NOTE: this assumes width is already properly scaled
                vector2d char_size = get_term_char_size();
                height -= height % char_size.y;
                if (height < char_size.y) height = char_size.y;
            }
        }
    }

    return vector2d(width, height);
}


vector2d ImgMan::calc_img_size_term_chars(vector2d img_size, int width, int height)
{
    vector2d char_size = get_term_char_size();
    vector2d calc_img_size = calc_img_size_pixels(img_size, width * char_size.x, height * char_size.y);
    calc_img_size.x = (float)calc_img_size.x / (float)char_size.x;
    calc_img_size.y = (float)calc_img_size.y / (float)char_size.y;
    return calc_img_size;
}


vector2d ImgMan::draw_img_term_coord_and_size(
    std::string path,
    int width,
    int height,
    int coord_x,
    int coord_y,
    vector4d crop
)
{
    img_data* image_data = IMG_MAN.img_cache_map.get(path);
    if (!image_data)
    {
        return vector2d();
    }

    return draw_img_term_coord_and_size(*image_data, path, width, height, coord_x, coord_y, crop);
}


vector2d ImgMan::draw_img_term_coord_and_size(
    img_data& image_data,
    std::string path,
    int width,
    int height,
    int coord_x,
    int coord_y,
    vector4d crop
)
{
    if (path.empty()) return vector2d();

    vector2d img_size = get_img_size(path);
    vector2d term_sz = get_term_size_in_chars();

    vector2d char_size = get_term_char_size();
    if (char_size.x < 1 || char_size.y < 1) return vector2d();

    int w = width * char_size.x;
    int h = height * char_size.y;
    int x = coord_x * char_size.x;
    int y = coord_y * char_size.y;
    crop.a *= char_size.x;
    crop.b *= char_size.y;
    crop.c *= char_size.x;
    crop.d *= char_size.y;
    vector2d size_drawn = draw_img_pixel_coord(image_data, path, w, h, x, y, crop);
    size_drawn.x = (float)size_drawn.x / (float)char_size.x;
    size_drawn.y = (float)size_drawn.y / (float)char_size.y;
    return size_drawn;
}


vector2d ImgMan::draw_img_pixel_coord(
    img_data& image_data,
    std::string path,
    int width,
    int height,
    int coord_x,
    int coord_y,
    vector4d crop
)
{
    if (path.empty()) return vector2d();

    int offset_x = 0;
    int offset_y = 0;
    vector2d img_size = calc_img_size_pixels(get_img_size(path), width, height);
    width = img_size.x;
    height = img_size.y;

    if (!image_data.valid_pixmap() ||
        // if image has changed size, rebuild pixmap
        width != image_data.width || height != image_data.height)
    {
        //Benchmark::start_ms();
        load_pixmap(image_data, width, height);
        //Benchmark::end_ms("Load Pixmap: ");
    }

    // to crop left: subtract n from width
    width -= crop.a;
    // to crop top: add n to y_offset and subtract n from height
    offset_y += crop.b;
    height -= crop.b;
    // to crop right: add n to x_offset and subtract n from width
    offset_x += crop.c;
    width -= crop.c;
    // to crop bottom: subtract n from height
    height -= crop.d;

    if (width < 1 || height < 1)
    {
        return vector2d();
    }

    // write image to display buffer
    if (image_data.valid_pixmap())
    {
        show_image(
            image_data,
            offset_x, offset_y,
            width, height,
            coord_x, coord_y);

        sixel_draw_buffer.emplace_back(
            &image_data,
            vector2d(offset_x, offset_y),
            vector2d(width, height),
            vector2d(coord_x, coord_y));

        return vector2d(width, height);
    }

    return vector2d();
}


bool ImgMan::load_pixmap(img_data& img, const int w, const int h)
{
    if (!xi.display || w < 1 || h < 1) return false;

    {
        // MUTEX
        std::unique_lock<std::shared_mutex> lck(IMG_MAN.imlib_mtx);

        img.set_pixmap(XCreatePixmap(xi.display, xi.parent, w, h, xi.depth));
                        //DefaultDepth(xi.display, 0)));
        if (!img.valid_pixmap()) goto error;

        // load from disk
        Imlib_Image im = imlib_load_image_without_cache(img.image_path.c_str());
        if (!im) goto error;    // das rite

        imlib_context_set_image(im);

        XSetForeground(xi.display, xi.imageGC, xi.background_pixel);
        XFillRectangle(xi.display, img.get_pixmap(), xi.imageGC, 0, 0, w, h);

        imlib_context_set_display(xi.display);
        imlib_context_set_visual(DefaultVisual(xi.display, 0));
        imlib_context_set_colormap(DefaultColormap(xi.display, 0));
        imlib_context_set_drawable((Drawable) img.get_pixmap());
        imlib_render_image_on_drawable_at_size(0, 0, w, h);

        imlib_free_image();

        img.width = w;
        img.height = h;

        return true;
    }

    error:

    img.invalidate_pixmap();

    return false;
}


bool ImgMan::show_image(img_data& img, int sx, int sy, int sw, int sh, int x, int y)
{
    if (!img.valid_pixmap()) return false;

    XCopyArea(xi.display, img.get_pixmap(), xi.window, xi.imageGC,
	      sx, sy,
	      (sw ? sw : img.width),
	      (sh ? sh : img.height), x + xi.offset_x, y + xi.offset_y);

    return true;
}


void ImgMan::redraw_buffer(bool b_sync)
{
    for (auto& sd : sixel_draw_buffer)
    {
        if (sd.image_data)
        {
            show_image(
                *sd.image_data,
                sd.offset.x, sd.offset.y,
                sd.size.x, sd.size.y,
                sd.coord.x, sd.coord.y
            );
        }
    }

    if (b_sync) sync();
}


void ImgMan::clear_term_cells(int x_0, int y_0, int x_1, int y_1)
{
    if (!xi.display) return;

    vector2d char_size = get_term_char_size();
    x_0 *= char_size.x;
    y_0 *= char_size.y;
    x_1 *= char_size.x;
    y_1 *= char_size.y;

    if (x_0 < 0) x_0 = 0;
    if (y_0 < 0) y_0 = 0;

    XClearArea(xi.display, xi.window, x_0, y_0, x_1, y_1, False);
}


void ImgMan::wipe_screen()
{
    if (!xi.display) return;

    vector2d win_size = xi.get_win_size();
    XClearArea(xi.display, xi.window, 0, 0, win_size.x, win_size.y, False);
}


// draw to screen
void ImgMan::sync()
{
    if (!xi.display) return;
    // NOTE! if the event queue is not discarded here by
    //       by passing true as the second argument to XSync(),
    //       then it will grow inifintely in size over time
    //       and use up all of the system's memory!
    XSync(xi.display, true /* discard all pending events */);
    // event queue size:
    //int l = XQLength(xi->display);
    //ERR(l, "X Queue: ");
}

