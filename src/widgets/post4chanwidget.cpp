/**
 * Comfy
 *
 *  Copyright 2019 by Wolfish <wolfish@airmail.cc>
 *  https://wolfish.neocities.org/soft/comfy/
 *
 *  Licensed under the GPL v2.0 only.
 */
#include "post4chanwidget.h"
#include "../netops.h"
#include "../widgetman.h"
#include "widgets.h"
#include <iomanip>


Post4chanWidget::Post4chanWidget(Thread4chanWidget* _thread, imageboard::post& _post_data, vector4d _padding, uint32_t _bg_color, uint32_t _fg_color) 
: TermWidget(vector2d(), _padding, vector4d(), _bg_color, _fg_color, false)
, thread(_thread)
, post_data(&_post_data)
{
    b_is_op = _post_data.b_op;
    post_num = _post_data.num;
    vbox = nullptr;
    info_hbox = nullptr;
    flag_box = nullptr;
    image_box = nullptr;
    post_info_box = nullptr;
    post_info = nullptr;
    post_text = nullptr;
    reply_div = nullptr;
    replies_text = nullptr;
    set_h_sizing(e_widget_sizing::ws_fill);
    set_v_sizing(e_widget_sizing::ws_auto);
}


std::vector<ImageWidget*> Post4chanWidget::get_image_widgets()
{
    std::vector<ImageWidget*> wgts;

    if (image_box)
    {
        ImageWidget* img =
            dynamic_cast<ImageWidget*>(image_box->get_child_widget().get());
        if (img) wgts.push_back(img);
    }

    if (flag_box)
    {
        ImageWidget* img =
            dynamic_cast<ImageWidget*>(flag_box->get_child_widget().get());
        if (img) wgts.push_back(img);
    }

    return wgts;
}


void Post4chanWidget::rebuild(bool b_rebuild_children)
{
    if (!thread || !post_data)
    {
        return;
    }

    if (!child_widget)
    {
        b_is_op = post_data->b_op;

        post_info =
            std::make_shared<TextWidget>(
                vector2d(),
                vector4d(1, 0, 1, 0),
                COLO.post_info_bg,
                COLO.post_info_fg);
        post_info->set_h_sizing(e_widget_sizing::ws_fill);
        post_info->set_v_sizing(e_widget_sizing::ws_dynamic);

        post_info_box =
            std::make_shared<BoxWidget>(
                vector2d(),                     // offset
                vector4d(0, 0, 0, 0),           // padding
                vector2d(1, 1),                 // size
                COLO.post_info_bg,              // bg_color
                COLO.post_info_bg);             // fg_color
        post_info_box->set_child_padding(vector4d());
        post_info_box->set_h_sizing(e_widget_sizing::ws_fill);
        post_info_box->set_v_sizing(e_widget_sizing::ws_auto);
        post_info_box->set_draw_border(false);
        post_info_box->set_child_widget(post_info, false);

        // poster name
        std::string nme = post_data->name;
        // tripcode
        if (!post_data->trip.empty())
        {
            nme += post_data->trip;
        }
        post_info->append_text(nme, false);

        // id
        if (!post_data->id.empty())
        {
            std::string pid = " (ID: ";
            pid += post_data->id;
            pid += ")";
            post_info->append_text(pid, false);
        }

        // post datetime
        std::string dte = " ";
        dte += post_data->datetime;
        post_info->append_text(dte, false);

        // post number
        std::string no = " No.";
        no += std::to_string(post_num);
        post_info->append_text(no, false);

        // image info
        if (!post_data->img_filename.empty())
        {
            bool b_kb = true;
            int bytes = post_data->img_fsize;
            float size = (float)bytes / 1024;
            if (size > 1024)
            {
                size = size / 1024;
                b_kb = false;
            }
            std::string inf = "File: " + post_data->img_filename;
            inf += post_data->img_ext;
            inf += " (";

            std::ostringstream size_str;
            size_str << std::fixed;
            size_str << std::setprecision(2);
            size_str << size;
            inf += size_str.str();
            if (b_kb)
            {
                inf += " KB";
            }
            else
            {
                inf += " MB";
            }

            inf += ", " + std::to_string(post_data->img_w);
            inf += "x" + std::to_string(post_data->img_h) + ")";
            img_info = std::make_shared<TextWidget>(
                vector2d(),
                vector4d(0, 0, 0, 1),
                COLO.post_bg, COLO.post_img_info);
            img_info->set_h_sizing(e_widget_sizing::ws_fill);
            img_info->set_v_sizing(e_widget_sizing::ws_dynamic);
            img_info->append_text(inf, false /* rebuild */);
        }

        // post text
        if (!post_data->text.empty())
        {
            post_text = std::make_shared<TextWidget>(
                vector2d(),
                vector4d(0, 0, 0, 1),
                COLO.post_bg, COLO.post_text);
            post_text->set_h_sizing(e_widget_sizing::ws_fill);
            post_text->set_v_sizing(e_widget_sizing::ws_dynamic);
            post_text->set_parse_4chan(true);
            post_text->append_text(post_data->text, false /* rebuild */);
        }

        main_box =
            std::make_shared<BoxWidget>(
                vector2d(),
                vector4d(),
                vector2d(),
                COLO.post_bg,
                COLO.post_border);
        main_box->set_h_sizing(e_widget_sizing::ws_fill);
        main_box->set_v_sizing(e_widget_sizing::ws_auto);

        if (DISPLAY_IMAGES)
        {
            // flag image
            if (!post_data->country.empty() ||
                !post_data->troll_country.empty())
            {
                std::string file_name;
                std::string flag_url;

                if (!post_data->country.empty())
                {
                    file_name = post_data->country;
                    str_to_lower(file_name);
                    file_name += ".gif";
                    flag_url = "s.4cdn.org/image/country/";
                    flag_url += file_name;
                }
                else
                {
                    file_name = post_data->troll_country;
                    str_to_lower(file_name);
                    file_name += ".gif";
                    flag_url = "s.4cdn.org/image/country/troll/";
                    flag_url += file_name;
                }

                IMG_MAN.request_image(
                    flag_url,
                    vector2d(-1, FLAG_IMG_H),
                    thread->get_id(),
                    thread->get_thread_num_str(),
                    post_num);

                // flag box
                flag_box = std::make_shared<BoxWidget>(
                    vector2d(), // offset
                    vector4d(0, 0, 0, 0) // padding
                );
                flag_box->set_size(3, FLAG_IMG_H);
                flag_box->set_h_sizing(e_widget_sizing::ws_fixed);
                flag_box->set_v_sizing(e_widget_sizing::ws_fixed);
                flag_box->set_child_padding(vector4d());
                flag_box->set_draw_border(false);
                flag_box->set_bg_color(COLO.post_bg);
                flag_box->set_bg_color(COLO.post_bg);
            }

            // post images
            if (post_data->img_time != 0)
            {
                std::string file_name =
                    std::to_string(post_data->img_time) +
                    post_data->img_ext;
                std::string img_url = "https://i.4cdn.org/";
                img_url += thread->get_board();
                img_url += "/" + file_name;

                IMG_MAN.request_image(
                    img_url,
                    vector2d(-1, POST_IMG_H),
                    thread->get_id(),
                    thread->get_thread_num_str(),
                    post_num);

                // image box
                image_box = std::make_shared<BoxWidget>(
                    vector2d(), // offset
                    vector4d(0, 0, 0, 1) // padding
                );
                image_box->set_size(1, POST_IMG_H);
                image_box->set_h_sizing(e_widget_sizing::ws_auto);
                image_box->set_v_sizing(e_widget_sizing::ws_fixed);
                image_box->set_child_padding(vector4d());
                image_box->set_draw_border(false);
                image_box->set_bg_color(COLO.post_bg);
                image_box->set_fg_color(COLO.post_bg);
            }
        }   // endif (DISPLAY_IMAGES)

        rebuild_vbox();
        set_child_widget(main_box, false /* rebuild */);
        main_box->rebuild(true);
        update_size(true /* recursive */);
    }
    else if (b_rebuild_children)
    {
        if (child_widget)
        {
            child_widget->rebuild(true);

            // image has undersized height
            if (image_box && image_box->get_child_widget() &&
                image_box->get_child_widget()->get_size().y !=
                image_box->get_size().y)
            {
                image_box->set_size(image_box->get_child_widget()->get_size());
                image_box->rebuild(false);
                rebuild_vbox();

                if (vbox)
                    vbox->rebuild(false);

                if (main_box)
                    main_box->rebuild(false);
            }
        }

        update_size(true /* recursive */);
    }
}


void Post4chanWidget::rebuild_vbox()
{
    if (!main_box) return;

    info_hbox = std::make_shared<HorizontalBoxWidget>(
        vector2d(), // offset
        vector4d(0, 0, 0, 1), // padding
        vector4d(0, 0, 1, 0) // child padding
    );
    info_hbox->set_h_sizing(e_widget_sizing::ws_fill);
    info_hbox->set_v_sizing(e_widget_sizing::ws_auto);
    info_hbox->add_child_widget(flag_box, false);
    info_hbox->add_child_widget(post_info_box, false);

    vbox = std::make_shared<VerticalBoxWidget>();
    vbox->add_child_widget(info_hbox, false);
    vbox->add_child_widget(img_info, false);
    vbox->add_child_widget(image_box, false);
    vbox->add_child_widget(post_text, false);
    vbox->add_child_widget(reply_div, false);
    vbox->add_child_widget(replies_text, false);

    main_box->set_child_widget(vbox, false /* rebuild */);
}


void Post4chanWidget::parse_quotes()
{
    if (!post_text || !thread)
    {
        return;
    }

    for (auto& word : post_text->get_term_words())
    {
        int replied_to = post_text->parse_post_num_quote(word.text);
        if (replied_to != -1)
        {
            thread->add_reply(replied_to, post_num);
        }
    }
}


void Post4chanWidget::load_replies()
{
    if (replies.size() < 1 || !main_box) return;

    reply_div = std::make_shared<BoxDividerWidget>(true /* horizontal */, COLO.post_bg, COLO.post_border);

    replies_text = std::make_shared<TextWidget>(vector2d(), vector4d(0, 0, 0, 0), COLO.post_bg, COLO.post_reply);
    replies_text->set_h_sizing(e_widget_sizing::ws_fill);
    replies_text->set_v_sizing(e_widget_sizing::ws_dynamic);
    replies_text->set_parse_4chan(true);

    for (auto& reply : replies)
    {
        replies_text->append_text(
            ">>" + std::to_string(reply),
            false  // rebuild
        );
    }

    rebuild_vbox();
}


bool Post4chanWidget::add_image(img_packet& pac, bool b_refresh_parent)
{
    if (!DISPLAY_IMAGES || !thread ||
        !main_box || !vbox || pac.is_video())
        return false;

    bool b_added = false;

    // flag
    if (pac.parser.url.find("s.4cdn.org/image/country/") != std::string::npos)
    {
        std::shared_ptr<ImageWidget> img = std::make_shared<ImageWidget>(
            pac,
            true, // maintain aspect ratio
            false, // fullscreen on click
            vector2d(3, FLAG_IMG_H) // size
        );
        img->set_h_sizing(e_widget_sizing::ws_fixed); // slave width to height
        img->set_v_sizing(e_widget_sizing::ws_dynamic);

        flag_box->set_fg_color(COLO.img_artifact_remove);
        flag_box->set_fg_color(COLO.img_artifact_remove);
        flag_box->set_child_widget(img, true /* rebuild */);

        b_refresh_parent = false;
        b_added = true;
    }
    // post image
    else
    {
        std::shared_ptr<ImageWidget> img = std::make_shared<ImageWidget>(
            pac,
            true, // maintain aspect ratio
            true, // fullscreen on click
            vector2d(-1, POST_IMG_H) // size
        );
        img->set_h_sizing(e_widget_sizing::ws_dynamic); // slave width to height
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
        img->rebuild();

        // image has undersized height
        if (img->get_size().y != main_box->get_size().y)
        {
            image_box->set_size(img->get_size());
        }
        else
        {
            b_refresh_parent = false;
        }

        image_box->rebuild(false);
        b_added = true;
    }

    if (b_refresh_parent &&
        thread->get_posts_vbox() &&
        thread->get_scroll_panel())
    {
        rebuild_vbox();
        vbox->rebuild(false);
        main_box->rebuild(false);
        update_size(false /* recursive */);
        thread->get_posts_vbox()->rebuild(false);
        thread->get_scroll_panel()->rebuild(false);
    }

    return b_added;
}


vector2d Post4chanWidget::get_child_widget_size() const
{
    if (child_widget)
    {
        return child_widget->get_size();
    }

    return vector2d();
}


void Post4chanWidget::update_child_size(bool b_recursive)
{
    if (child_widget)
    {
        child_widget->update_size(b_recursive);
    }
}


void Post4chanWidget::select()
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


void Post4chanWidget::unselect()
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


void Post4chanWidget::receive_left_click(vector2d coord, TermWidget* clicked, TermWidget* source)
{
    if (!thread) return;

    TextWidget* text = dynamic_cast<TextWidget*>(clicked);
    if (text)
    {
        std::wstring word = text->get_word_at_coord(coord);
        int post_num = text->parse_post_num_quote(word);
        if (post_num != -1)
        {
            thread->scroll_to_post(post_num);
        }
    }
    else
    {
        thread->select_post(this);
    }
}

