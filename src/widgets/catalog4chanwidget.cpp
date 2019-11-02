/**
 * Comfy
 *
 *  Copyright 2019 by Wolfish <wolfish@airmail.cc>
 *  https://wolfish.neocities.org/soft/comfy/
 *
 *  Licensed under the GPL v2.0 only.
 */
#include "catalog4chanwidget.h"
#include "../netops.h"
#include "../fileops.h"
#include "../widgetman.h"
#include "widgets.h"


Catalog4chanWidget::Catalog4chanWidget(data_4chan& chan_data, bool b_update)
: Thread4chanWidget(chan_data, false /* prevent base class update() */)
{
    board_url = chan_data.parser.url;
    auto_ref_s = 60;    // seconds
    auto_refresh_interval = std::chrono::milliseconds(auto_ref_s * 1000);
    selected_thread = nullptr;
    b_can_save = false;

    title = "/" + board + "/ - Catalog";

    // size of thread boxes in grid display
    thread_box_size = vector2d(36, 25);

    if (b_update)
    {
        update(chan_data);
    }
}


bool Catalog4chanWidget::on_received_update(data_4chan& chan_data)
{
    b_reloading = false;
    auto_refresh_counter = std::chrono::milliseconds(0);

    bool b_redraw = false;
    for (auto& thread : chan_data.page_data->posts)
    {
        std::shared_ptr<CatalogThread4chanWidget> cat_thread =
            get_thread(thread.num);
        if (cat_thread)
        {
            bool b_changed = cat_thread->update_reply_and_image_count(thread);
            if (b_changed)
            {
                b_redraw = true;
            }
        }
    }

    return b_redraw;
}


void Catalog4chanWidget::rebuild(bool b_rebuild_children)
{
    if (!page_data) return;

    vector2d cached_scroll_pos;

    if (scroll_panel)
    {
        cached_scroll_pos = scroll_panel->get_scroll_position();
    }

    // main vertical box container
    std::shared_ptr<VerticalBoxWidget> main_vbox = std::make_shared<VerticalBoxWidget>(false /* b_stretch_offscreen */);
    main_vbox->set_h_sizing(e_widget_sizing::ws_fullscreen);
    main_vbox->set_v_sizing(e_widget_sizing::ws_fullscreen);

    // threads box
    // note: scroll panels must be put in a container box
    // to give them a fixed size on screen
    posts_box =
        std::make_shared<BoxWidget>(
            vector2d(),         // offset
            vector4d(),         // padding
            vector2d(1, 1),     // size
            COLO.thread_bg,     // bg_color
            COLO.thread_bg);    // fg_color

    posts_box->set_child_padding(vector4d());
    posts_box->set_h_sizing(e_widget_sizing::ws_fullscreen);
    posts_box->set_v_sizing(e_widget_sizing::ws_fixed);
    posts_box->set_draw_border(false);

    // posts grid container
    std::shared_ptr<WrapGrid> threads_grid =
        std::make_shared<WrapGrid>(
            thread_box_size /* grid slot size */);
    threads_grid->set_h_align(e_widget_align::wa_center);
    threads_grid->set_v_align(e_widget_align::wa_top);

    // posts scroll panel
    scroll_panel =
        std::make_shared<ScrollPanelWidget>(
            true,
            vector2d(),
            true,
            false,
            COLO.scroll_bar_bg,
            COLO.scroll_bar_fg);

    scroll_panel->set_h_sizing(e_widget_sizing::ws_fill);
    scroll_panel->set_v_sizing(e_widget_sizing::ws_auto);
    scroll_panel->set_child_widget(threads_grid, false /* rebuild */);

    posts_box->set_child_widget(scroll_panel, false);

    // header bar
    header =
        std::make_shared<BoxWidget>(
            vector2d(),     // offset
            vector4d(),     // padding
            vector2d(1, 1), // size
            COLO.catalog_header_bg,
            COLO.catalog_header_bg);

    header->set_child_padding(vector4d());
    header->set_h_sizing(e_widget_sizing::ws_fullscreen);
    header->set_v_sizing(e_widget_sizing::ws_auto);
    header->set_draw_border(false);

    header_info =
        std::make_shared<TextWidget>(
            vector2d(),
            vector4d(1, 0, 0, 0),
            COLO.catalog_header_bg,
            COLO.catalog_header_fg);

    header_info->set_h_sizing(e_widget_sizing::ws_fill);
    header_info->set_v_sizing(e_widget_sizing::ws_dynamic);
    header_info->append_text("4chan /" + board + "/ - Catalog", false);
    header->set_child_widget(header_info);

    // footer bar
    footer =
        std::make_shared<BoxWidget>(
            vector2d(),     // offset
            vector4d(),     // padding
            vector2d(1, 1), // size
            COLO.catalog_footer_bg, 
            COLO.catalog_footer_fg);

    //footer->set_inherited_offset(vector2d(0, term_h() - 1));
    footer->set_child_padding(vector4d());
    footer->set_h_sizing(e_widget_sizing::ws_fullscreen);
    footer->set_v_sizing(e_widget_sizing::ws_auto);
    footer->set_draw_border(false);

    footer_info =
        std::make_shared<TextWidget>(
            vector2d(),
            vector4d(1, 0, 0, 0),
            COLO.catalog_footer_bg,
            COLO.catalog_footer_fg);

    footer_info->set_h_sizing(e_widget_sizing::ws_fill);
    footer_info->set_v_sizing(e_widget_sizing::ws_dynamic);

    footer->set_child_widget(footer_info);

    // load threads
    std::vector<int> keys;
    std::vector<int> new_keys;
    for (auto const& t : thread_map)
    {
        // collect keys of existing
        keys.push_back(t.first);
        // remove image artifacts of existing
        if (t.second)
        {
            std::vector<ImageWidget*> imgs = t.second->get_image_widgets();
            for (auto& i : imgs)
            {
                if (i)
                {
                    i->blast_out_image_artifacts_at_last_position();
                }
            }
        }
    }

    for (auto& thread : page_data->posts)
    {
        std::shared_ptr<CatalogThread4chanWidget> cat_thread =
            get_thread(thread.num);
        if (!cat_thread)
        {
            cat_thread =
                std::make_shared<CatalogThread4chanWidget>(
                    this,
                    thread);
            thread_map[cat_thread->get_post_num()] = cat_thread;
        }

        new_keys.push_back(cat_thread->get_post_num());
        threads_grid->add_child_widget(cat_thread, false);
    }

    // remove old threads
    for (auto& k : keys)
    {
        auto it = std::find(new_keys.begin(), new_keys.end(), k);
        if (it == new_keys.end())
        {
            thread_map.erase(k);
        }
    }

    footer_info->append_text("Threads:");
    footer_info->append_text(std::to_string(thread_map.size()));

    base_footer_text = footer_info->get_term_words();
    if (!footer_countdown.empty())
    {
        footer_info->append_text(footer_countdown);
    }
    else
    {
        std::string t = "| Reload in " + std::to_string(auto_ref_s) + "s";
        footer_info->append_text(t);
        b_auto_update = true;
    }

    main_vbox->add_child_widget(header, false /* rebuild */);
    main_vbox->add_child_widget(posts_box, false /* rebuild */);
    main_vbox->add_child_widget(footer, false /* rebuild */);

    // shrink posts_box to fit header and footer
    header->rebuild(true);
    footer->rebuild(true);
    int shrink = header->get_height_constraint();
    shrink += footer->get_height_constraint();
    posts_box->set_size(1, term_h() - shrink);

    set_child_widget(main_vbox, false /* rebuild */);
    main_vbox->rebuild(true /* recursive */);

    // restore thread scroll position
    // (must be done after being built, as it requires
    // the inherited_offset to be set by its parent)
    scroll_panel->set_scroll_position(cached_scroll_pos);
    scroll_panel->rebuild(false);

    // sets term size cache
    update_size(false);
}


std::shared_ptr<CatalogThread4chanWidget> Catalog4chanWidget::get_thread(int post_num)
{
    try
    {
        std::shared_ptr<CatalogThread4chanWidget> wgt = thread_map.at(post_num);
        if (wgt)
        {
            return wgt;
        }
    }
    catch (const std::out_of_range& oor)
    {
        return nullptr;
    }

    return nullptr;
}


bool Catalog4chanWidget::receive_img_packet(img_packet& pac)
{
    std::shared_ptr<CatalogThread4chanWidget> thread = get_thread(pac.post_key);
    if (thread)
    {
        return thread->add_image(pac, false /* refresh thread */);
    }

    return false;
}


void Catalog4chanWidget::select_thread(CatalogThread4chanWidget* thread)
{
    if (!thread) return;

    if (thread != selected_thread)
    {
        if (selected_thread) selected_thread->unselect();
        thread->select();
        selected_thread = thread;
    }
    else
    {
        thread->unselect();
        selected_thread = nullptr;
    }
}


// TODO:    enable navigation of threads using key input
//
//bool Catalog4chanWidget::handle_key_input(const tb_event& input_event)
//{
//    bool b_handled = Thread4chanWidget::handle_key_input(input_event);
//    if (!b_handled)
//    {
//        if (input_event.key == TB_KEY_...)
//        {
//        }
//    }
//
//    return b_handled;
//}


void Catalog4chanWidget::receive_left_click(vector2d coord, TermWidget* clicked, TermWidget* source)
{
}

