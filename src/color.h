/**
 * Comfy
 *
 *  Copyright 2019 by Wolfish <wolfish@airmail.cc>
 *  https://wolfish.neocities.org/soft/comfy/
 *
 *  Licensed under the GPL v2.0 only.
 */
#include <cstdint>

namespace Colors
{
    struct color_scheme
    {
        // color used as default term background
        uint8_t app_bg;

        // title
        uint8_t title_bg;
        uint8_t title_fg;
        uint8_t motd;

        // info displayed above thread
        uint8_t thread_header_bg;
        uint8_t thread_header_fg;
        uint8_t thread_saved;
        uint8_t thread_archived;

        // info displayed below thread
        uint8_t thread_footer_bg;
        uint8_t thread_footer_fg;
        
        // info displayed above catalog
        uint8_t catalog_header_bg;
        uint8_t catalog_header_fg;

        // info displayed below catalog
        uint8_t catalog_footer_bg;
        uint8_t catalog_footer_fg;

        // scroll bar
        uint8_t scroll_bar_bg;
        uint8_t scroll_bar_fg;

        // full screen image
        uint8_t fs_image_bg;
        uint8_t fs_image_header_bg;
        uint8_t fs_image_header_fg;

        // catalog page
        uint8_t thread_info;
        uint8_t thread_reply_count;
        uint8_t thread_image_count;
        uint8_t thread_title;
        uint8_t thread_text;
        uint8_t thread_bg;
        uint8_t thread_border;

        // posts in a thread
        uint8_t post_bg;
        uint8_t post_text;
        uint8_t post_border;
        uint8_t post_info_bg;
        uint8_t post_info_fg;
        uint8_t post_img_info;
        uint8_t post_reply;         // e.g. ">>423423777"
        uint8_t post_greentext;     // e.g. ">foo bar baz"

        uint8_t selection_bg;
        uint8_t selection_fg;

        uint8_t switch_widget_bg;
        uint8_t switch_widget_fg;
        uint8_t switch_widget_border;

        // this color is used to wipe the screen
        // of image artifacts after an image has
        // change position: it MUST be a different
        // color than the background it is appearing on,
        // otherwise removing image artifacts WILL NOT WORK.
        // choose a color that is unused in this color scheme,
        // and for best results set this color to one that is
        // very close to the post background color
        // (such as the color set to post_bg) so that
        // it's less noticeable
        uint8_t img_artifact_remove;
    };


    /* ------------ COLOR SCHEMES ------------ */

    // good color cheat sheet here:
    // https://jonasjacek.github.io/colors/
    //
    // note: avoid using colors 0 - 15, as many
    //       term emulators will set custom colors
    //       to those values (e.g. 0 will be white
    //       instead of black)

    static const color_scheme COMFYBLUE
    {
        app_bg:                 16,

        title_bg:               17,
        title_fg:               87,
        motd:                   255,

        thread_header_bg:       16,
        thread_header_fg:       255,
        thread_saved:           47,
        thread_archived:        203,

        thread_footer_bg:       16,
        thread_footer_fg:       255,

        catalog_header_bg:      16,
        catalog_header_fg:      255,

        catalog_footer_bg:      16,
        catalog_footer_fg:      255,

        scroll_bar_bg:          17,
        scroll_bar_fg:          87,

        fs_image_bg:            16,
        fs_image_header_bg:     16,
        fs_image_header_fg:     255,

        thread_info:            255,
        thread_reply_count:     255,
        thread_image_count:     255,
        thread_title:           203,
        thread_text:            87,
        thread_bg:              17,
        thread_border:          87,

        post_bg:                17,
        post_text:              87,
        post_border:            87,
        post_info_bg:           87,
        post_info_fg:           17,
        post_img_info:          255,
        post_reply:             255,
        post_greentext:         47,

        selection_bg:           17,
        selection_fg:           87,

        switch_widget_bg:       17,
        switch_widget_fg:       87,
        switch_widget_border:   255,

        img_artifact_remove:    16,
    };
};

