/**
 * Comfy
 *
 *  Copyright 2019 by Wolfish <wolfish@airmail.cc>
 *  https://wolfish.neocities.org/soft/comfy/
 *
 *  Licensed under the GPL v2.0 only.
 */
#include "catalogthread4chanwidget.h"
#include "../netops.h"
#include "../widgetman.h"
#include "widgets.h"


CatalogThread4chanWidget::CatalogThread4chanWidget(Catalog4chanWidget* _catalog, imageboard::post& _post_data, vector4d _padding, uint32_t _bg_color, uint32_t _fg_color) 
: Post4chanWidget(nullptr, _post_data, _padding, _bg_color, _fg_color)
, catalog(_catalog)
{
    post_title = nullptr;
    set_h_sizing(e_widget_sizing::ws_auto);
    set_v_sizing(e_widget_sizing::ws_auto);
    img_size = vector2d(32, 15);
}


void CatalogThread4chanWidget::rebuild(bool b_rebuild_children)
{
    if (!catalog || !post_data)
    {
        return;
    }

    if (!child_widget)
    {
        std::string sub = post_data->subject;
        std::string com = post_data->text;
        std::string ext = post_data->img_ext;
        int64_t tim = post_data->img_time;
        int no = post_data->num;
        reply_count = post_data->replies;
        image_count = post_data->images;

        post_num = no;

        post_info = std::make_shared<TextWidget>(
            vector2d(),
            vector4d(0, 0, 0, 1),
            COLO.thread_bg, COLO.thread_info);
        post_info->set_h_align(e_widget_align::wa_center);
        post_info->set_v_align(e_widget_align::wa_top);
        post_info->append_text("R:", false, false, false, false, COLO.thread_bg, COLO.thread_info);
        post_info->append_text(std::to_string(reply_count), false, true, false, false, COLO.thread_bg, COLO.thread_reply_count);
        post_info->append_text(" I:", false, false, false, false, COLO.thread_bg, COLO.thread_info);
        post_info->append_text(std::to_string(image_count), false, true, false, false, COLO.thread_bg, COLO.thread_image_count);

        if (!sub.empty())
        {
            post_title = std::make_shared<TextWidget>(
                vector2d(),
                vector4d(0, 0, 0, 1),
                COLO.thread_bg, COLO.thread_title);
            post_title->set_h_sizing(e_widget_sizing::ws_fill);
            post_title->set_v_sizing(e_widget_sizing::ws_dynamic);
            post_title->append_text(
                sub,
                false,  // rebuild
                true);  // bold
        }

        post_text = std::make_shared<TextWidget>(
            vector2d(),
            vector4d(0, 0, 0, 1),
            COLO.thread_bg, COLO.thread_text);
        post_text->set_h_sizing(e_widget_sizing::ws_fill);
        post_text->set_v_sizing(e_widget_sizing::ws_fill);
        post_text->set_parse_4chan(true);
        post_text->append_text(com, false /* rebuild */);

        main_box =
            std::make_shared<BoxWidget>(
                vector2d(),             // offset
                vector4d(),             // padding
                vector2d(),             // size
                COLO.thread_bg,         // bg_color
                COLO.thread_border);    // fg_color
        // auto-size to this widget's size, which is set
        // automatiaclly by the WrapGrid it is contained in
        // in the Catalog4chanWidget (the grid slot size of the WrapGrid)
        main_box->set_h_sizing(ws_fill);
        main_box->set_v_sizing(ws_fill);

        // image
        if (DISPLAY_IMAGES)
        {
            // add 's.jpg' to get the thumbnail version of the image
            std::string file_name = std::to_string(tim) + "s.jpg";
            std::string img_url = "https://i.4cdn.org/";
            img_url += catalog->get_board();
            img_url += "/" + file_name;

            IMG_MAN.request_image(
                img_url,
                img_size,
                catalog->get_id(),
                std::to_string(post_num),
                post_num);

            // image box
            image_box = std::make_shared<BoxWidget>(
                vector2d(), // offset
                vector4d(0, 0, 0, 1) // padding
            );
            image_box->set_h_align(e_widget_align::wa_center);
            image_box->set_v_align(e_widget_align::wa_top);
            image_box->set_h_sizing(e_widget_sizing::ws_auto);
            image_box->set_v_sizing(e_widget_sizing::ws_auto);
            image_box->set_child_padding(vector4d());
            image_box->set_draw_border(false);
            image_box->set_bg_color(COLO.thread_bg);
            image_box->set_fg_color(COLO.thread_bg);
        }

        rebuild_vbox();
        set_child_widget(main_box, false /* rebuild */);
        main_box->rebuild(true);
        update_size(true);
    }
    else if (b_rebuild_children)
    {
        if (child_widget)
        {
            child_widget->rebuild(true);
        }

        update_size(true /* recursive */);
    }
}


bool CatalogThread4chanWidget::update_reply_and_image_count(imageboard::post& _post_data)
{
    if (_post_data.replies == reply_count &&
        _post_data.images == image_count)
    {
        return false;
    }

    reply_count = _post_data.replies;
    image_count = _post_data.images;

    post_info = std::make_shared<TextWidget>(
        vector2d(),
        vector4d(0, 0, 0, 1),
        COLO.thread_bg, COLO.thread_info);
    post_info->set_h_align(e_widget_align::wa_center);
    post_info->set_v_align(e_widget_align::wa_top);
    post_info->append_text("R:", false, false, false, false, COLO.thread_bg, COLO.thread_info);
    post_info->append_text(std::to_string(reply_count), false, true, false, false, COLO.thread_bg, COLO.thread_reply_count);
    post_info->append_text("I:", false, false, false, false, COLO.thread_bg, COLO.thread_info);
    post_info->append_text(std::to_string(image_count), false, true, false, false, COLO.thread_bg, COLO.thread_image_count);

    rebuild_vbox();

    return true;
}


void CatalogThread4chanWidget::rebuild_vbox()
{
    if (!main_box) return;

    vbox = std::make_shared<VerticalBoxWidget>();
    vbox->add_child_widget(image_box, false);
    vbox->add_child_widget(post_info, false);
    vbox->add_child_widget(post_title, false);
    vbox->add_child_widget(post_text, false);

    main_box->set_child_widget(vbox, false /* rebuild */);
}


bool CatalogThread4chanWidget::add_image(img_packet& pac, bool b_refresh_parent)
{
    if (!DISPLAY_IMAGES || !catalog ||
        !main_box || pac.is_video())
        return false;

    std::shared_ptr<ImageWidget> img = std::make_shared<ImageWidget>(
        pac,
        true, // maintain aspect ratio
        false, // fullscreen on click
        img_size
    );
    img->set_h_sizing(e_widget_sizing::ws_fixed);
    img->set_v_sizing(e_widget_sizing::ws_fixed);

    // images must be contained within a box.
    // the box always stays the same size as the
    // uncropped image, which prevents the layout
    // from getting screwed up when images
    // auto crop when going off screen
    // or scrolling within a widget

    image_box->set_bg_color(COLO.img_artifact_remove);
    image_box->set_fg_color(COLO.img_artifact_remove);
    image_box->set_child_widget(img, false /* rebuild */);

    vbox->rebuild(true);

    return true;
}


void CatalogThread4chanWidget::select()
{
    b_selected = true;

    // causes terrible image flickering that seems
    // to be caused by the border characters changing,
    // and there doesn't seem to be any way to prevent
    // it from happening, unfortunately.
    //
    //if (reply_div)
    //{
    //    reply_div->set_fg_color(201);
    //    reply_div->rebuild();
    //}

    //if (main_box)
    //{
    //    main_box->set_fg_color(201);
    //    main_box->rebuild(false);
    //}

    //WIDGET_MAN.draw_widgets();
}


void CatalogThread4chanWidget::unselect()
{
    b_selected = false;

    // causes terrible image flickering that seems
    // to be caused by the border characters changing,
    // and there doesn't seem to be any way to prevent
    // it from happening, unfortunately.
    //
    //if (reply_div)
    //{
    //    reply_div->set_fg_color(87);
    //    reply_div->rebuild();
    //}

    //if (main_box)
    //{
    //    main_box->set_fg_color(87);
    //    main_box->rebuild(false);
    //}

    //WIDGET_MAN.draw_widgets();
}


void CatalogThread4chanWidget::receive_left_click(vector2d coord, TermWidget* clicked, TermWidget* source)
{
    //if (catalog)
    //{
    //    catalog->select_thread(this);
    //}

    if (!catalog) return;

    std::string url = "a.4cdn.org/" + catalog->get_board() + "/thread/";
    url += std::to_string(post_num) + ".json";
    WIDGET_MAN.open_thread(url, true /* b_steal_focus */);
}

