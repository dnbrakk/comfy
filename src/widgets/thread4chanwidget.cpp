/**
 * Comfy
 *
 *  Copyright 2019 by Wolfish <wolfish@airmail.cc>
 *  https://wolfish.neocities.org/soft/comfy/
 *
 *  Licensed under the GPL v2.0 only.
 */
#include "thread4chanwidget.h"
#include "../netops.h"
#include "../fileops.h"
#include "../widgetman.h"
#include "widgets.h"


Thread4chanWidget::Thread4chanWidget(data_4chan& chan_data, bool b_update)
: ChanWidget(vector2d(), vector4d(), vector4d(), COLO.post_bg, COLO.post_text, false)
{
    set_id(chan_data.parser.url);
    set_h_sizing(e_widget_sizing::ws_fill);
    set_v_sizing(e_widget_sizing::ws_auto);
    thread_url = chan_data.parser.url;
    board = chan_data.parser.board;
    thread_num_str = chan_data.parser.thread_num_str;
    b_ticks = true;
    set_tick_rate(1000 / 30);
    auto_ref_s = 10;    // seconds
    time_cache = -1;
    auto_refresh_interval = std::chrono::milliseconds(auto_ref_s * 1000);
    auto_refresh_counter = std::chrono::milliseconds(0);
    reload_flash_interval = std::chrono::milliseconds(333);
    reload_flash_num = 5;
    reload_flash_count = reload_flash_num;
    b_reload_flash_sym = false;
    selected_post = nullptr;

    header = nullptr;
    header_info = nullptr;
    footer = nullptr;
    footer_info = nullptr;
    posts_box = nullptr;
    scroll_panel = nullptr;

    b_reloading = false;
    b_archived = false;
    b_auto_update = false;
    b_manual_update = false;
    b_can_save = true;

    if (b_update)
    {
        update(chan_data);
    }
}


bool Thread4chanWidget::on_received_update(data_4chan& chan_data)
{
    b_reloading = false;
    auto_refresh_counter = std::chrono::milliseconds(0);

    // no redraw required
    return false;
}


void Thread4chanWidget::tick_event(std::chrono::milliseconds delta)
{
    using namespace std::chrono;

    if ((!b_auto_update && !b_manual_update) ||
        b_archived || !footer_info || !footer)
    {
        return;
    }

    bool b_redraw = false;
    auto_refresh_counter += delta;

    // flash circle
    if (b_reloading || reload_flash_count < reload_flash_num)
    {
        if (auto_refresh_counter >= reload_flash_interval)
        {
            auto_refresh_counter = milliseconds(0);

            // make last flash hold until countdown restarts
            if (reload_flash_count < reload_flash_num - 1)
            {
                footer_info->clear_text();
                footer_info->append_text(base_footer_text);
                footer_countdown = L"| Reloading ";
                std::wstring sym = L"\u25C9";   // (@)
                if (b_reload_flash_sym)
                {
                    sym = L"\u25CB";    // ( )
                }
                footer_countdown += sym;
                footer_info->append_text(footer_countdown, true);
            }

            b_reload_flash_sym = !b_reload_flash_sym;
            reload_flash_count++;
            b_redraw = true;

            if (!b_reloading && reload_flash_count >= reload_flash_num)
            {
                b_manual_update = false;
                if (!b_auto_update)
                {
                    footer_info->clear_text();
                    footer_info->append_text(base_footer_text);
                    footer_countdown = L"| Reload [paused]";
                    footer_info->append_text(footer_countdown, true);
                }
            }
            else if (b_reloading && reload_flash_count >= reload_flash_num)
            {
                reload_flash_count = 0;
                b_reload_flash_sym = !b_reload_flash_sym;
            }
        }
    }
    // auto refresh
    else if (auto_refresh_counter >= auto_refresh_interval)
    {
        auto_refresh_counter = reload_flash_interval;
        reload_flash_count = 0;
        b_reload_flash_sym = true;

        // reload from url
        // note: reloads must always set steal focus to false
        NetOps::http_get__4chan_json(
            thread_url,
            get_id(),
            false,  // steal focus
            last_update_time,
            get_id());

        b_reloading = true;
    }
    // countdown timer
    else
    {
        footer_info->clear_text();
        footer_info->append_text(base_footer_text);
        uint32_t time = auto_refresh_counter.count();
        time /= 1000;
        time = (auto_refresh_interval.count() / 1000) - time;
        footer_countdown = L"| Reload in ";
        footer_countdown += std::to_wstring(time);
        footer_countdown += L"s";
        footer_info->append_text(footer_countdown, true);

        if (time != time_cache)
            b_redraw = true;

        time_cache = time;
    }

    if (b_redraw)
        WIDGET_MAN.draw_widgets(
            footer.get(),
            false /* clear cells */,
            false /* clear images */);
}


void Thread4chanWidget::refresh(bool b_draw_term)
{
    if (child_widget)
    {
        update_child_size(true);
        child_widget->rebuild(true);

        if (!hidden())
        {
            draw_children();

            if (b_draw_term)
            {
                WIDGET_MAN.draw_widgets();
            }
        }
    }
}


void Thread4chanWidget::handle_term_resize_event()
{
    rebuild();
}


void Thread4chanWidget::rebuild(bool b_rebuild_children)
{
    if (!page_data || page_data->posts.size() < 1) return;

    vector2d cached_scroll_pos;

    if (scroll_panel)
    {
        cached_scroll_pos = scroll_panel->get_scroll_position();
    }

    // main vertical box container
    main_vbox = std::make_shared<VerticalBoxWidget>(false /* b_stretch_offscreen */);
    main_vbox->set_h_sizing(e_widget_sizing::ws_fullscreen);
    main_vbox->set_v_sizing(e_widget_sizing::ws_fullscreen);

    // posts box
    // note: scroll panels must be put in a container box
    // to give them a fixed size on screen
    posts_box =
        std::make_shared<BoxWidget>(
            vector2d(),     // offset
            vector4d(),     // padding
            vector2d(1, 1), // size
            COLO.post_bg,   // bg_color
            COLO.post_bg);  // fg_color

    posts_box->set_child_padding(vector4d());
    posts_box->set_h_sizing(e_widget_sizing::ws_fullscreen);
    posts_box->set_v_sizing(e_widget_sizing::ws_fixed);
    posts_box->set_draw_border(false);

    // posts vertical box container
    posts_vbox = std::make_shared<VerticalBoxWidget>();

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
    scroll_panel->set_child_widget(posts_vbox, false /* rebuild */);

    posts_box->set_child_widget(scroll_panel, false);

    // header bar
    header =
        std::make_shared<BoxWidget>(
            vector2d(),     // offset
            vector4d(),     // padding
            vector2d(1, 1), // size
            COLO.thread_header_bg,
            COLO.thread_header_fg);

    header->set_child_padding(vector4d());
    header->set_h_sizing(e_widget_sizing::ws_fullscreen);
    header->set_v_sizing(e_widget_sizing::ws_auto);
    header->set_draw_border(false);

    header_info =
        std::make_shared<TextWidget>(
            vector2d(),
            vector4d(1, 0, 0, 0),
            COLO.thread_header_bg,
            COLO.thread_header_fg);

    header_info->set_h_sizing(e_widget_sizing::ws_fill);
    header_info->set_v_sizing(e_widget_sizing::ws_dynamic);
    header_info->append_text("4chan /" + board + "/", false);
    header->set_child_widget(header_info);

    // footer bar
    footer =
        std::make_shared<BoxWidget>(
            vector2d(),     // offset
            vector4d(),     // padding
            vector2d(1, 1), // size
            COLO.thread_footer_bg, 
            COLO.thread_footer_fg);

    footer->set_child_padding(vector4d());
    footer->set_h_sizing(e_widget_sizing::ws_fullscreen);
    footer->set_v_sizing(e_widget_sizing::ws_auto);
    footer->set_draw_border(false);

    footer_info =
        std::make_shared<TextWidget>(
            vector2d(),
            vector4d(1, 0, 0, 0),
            COLO.thread_footer_bg, 
            COLO.thread_footer_fg);

    footer_info->set_h_sizing(e_widget_sizing::ws_fill);
    footer_info->set_v_sizing(e_widget_sizing::ws_dynamic);

    footer->set_child_widget(footer_info);

    // load post data
    std::vector<std::shared_ptr<Post4chanWidget>> post_vec;
    std::vector<std::shared_ptr<Post4chanWidget>> new_posts;

    for (auto& p : page_data->posts)
    {
        // post already exists
        // note: we are reusing existing posts instead of
        //       building new ones each time so that
        //       the image data won't be dumped
        //       and therefore need to be loaded
        //       from disk again
        //
        // TODO: handle case of deleted post or post image
        //
        std::shared_ptr<Post4chanWidget> post = get_post(p.num);

        // create new post
        if (!post)
        {
            post = std::make_shared<Post4chanWidget>(this, p, vector4d(), COLO.post_bg, COLO.post_text);
            post_map[post->get_post_num()] = post;
            new_posts.push_back(post);
        }

        if (p.b_op)
        {
            // subject
            thread_subject = p.subject;
            header_info->append_text("- " + thread_subject, false);
            base_header_text = header_info->get_term_words();

            // widget title
            title = "/" + board + "/ - ";
            if (!thread_subject.empty())
            {
                title += thread_subject;
            }
            else
            {
                std::string com = p.text;
                com.erase(
                    std::remove_if(
                        com.begin(),
                        com.end(),
                        [](char c){
                            return (c == '\n' || c == '\r' ||
                                    c == '\t' || c == '\v' || c == '\f');
                        }),
                    com.end());

                if (com.length() > 36)
                {
                    title += com.substr(0, 36);
                    title += "...";
                }
                else
                {
                    title += com.substr(0, com.length());
                }
            }

            // replies
            footer_info->append_text("Replies:");
            footer_info->append_text(
                std::to_string(p.replies).c_str());

            // images
            footer_info->append_text("| Images:");
            footer_info->append_text(
                std::to_string(p.images).c_str());

            // archived or not
            if (p.b_archived)
            {
                b_archived = true;
            }

            // unique ips
            footer_info->append_text("| IPs:");
            footer_info->append_text(
                std::to_string(p.unique_ips).c_str());
        }

        post_vec.push_back(post);
        posts_vbox->add_child_widget(post, false /* rebuild */);
    }

    // posts must be built before replies can be parsed and loaded
    posts_vbox->rebuild(true);

    // find all quotes (e.g. >>38938922) and add
    // them to the post that is being quoted
    for (auto& post : new_posts)
    {
        post->parse_quotes();
    }

    // add the quote replies to the post
    for (auto& post : post_vec)
    {
        post->load_replies();
    }

    // append "[Archived]" and "[Saved]" etc.
    update_header_info();

    base_footer_text = footer_info->get_term_words();
    if (!b_archived)
    {
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


std::shared_ptr<Post4chanWidget> Thread4chanWidget::get_post(int post_num)
{
    try
    {
        std::shared_ptr<Post4chanWidget> wgt = post_map.at(post_num);
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


void Thread4chanWidget::add_reply(int post, int reply)
{
    std::shared_ptr<Post4chanWidget> wgt = get_post(post);
    if (wgt)
    {
        wgt->add_reply(reply);
    }
}


bool Thread4chanWidget::receive_img_packet(img_packet& pac)
{
    std::shared_ptr<Post4chanWidget> post = get_post(pac.post_key);
    if (post)
    {
        return post->add_image(pac, true /* refresh thread if needed */);
    }

    return false;
}


void Thread4chanWidget::on_focus_received()
{
    // only update size and rebuild if term size has changed
    if (term_size_cache != vector2d(term_w(), term_h()))
    {
        update_size(false);
        rebuild();
    }

    show();

    WIDGET_MAN.termbox_clear(COLO.img_artifact_remove);
    WIDGET_MAN.termbox_draw();
    WIDGET_MAN.draw_widgets();
}


bool Thread4chanWidget::handle_key_input(const tb_event& input_event, bool b_bubble_up)
{
    bool b_handled = false;

    // reload thread
    if (input_event.key == TB_KEY_CTRL_R)
    {
        if (!b_reloading)
        {
            auto_refresh_counter = std::chrono::milliseconds(0);
            reload_flash_count = 0;
            b_reload_flash_sym = true;
            b_reloading = true;
            b_manual_update = true;

            // note: reloads must always set steal focus to false
            NetOps::http_get__4chan_json(
                thread_url,
                get_id(),
                false,  // steal focus
                last_update_time,
                get_id());

            WIDGET_MAN.draw_widgets();
        }

        b_handled = true;
    }
    // mark or unmark thread dir for saving
    else if (b_can_save && input_event.key == TB_KEY_CTRL_S)
    {
        if (!is_saved())
        {
            save_to_disk();
        }
        else
        {
            delete_save_file();
        }

        update_header_info();

        int shrink = 0;
        if (header)
        {
            header->rebuild(true);
            shrink = header->get_height_constraint();
        }

        if (footer)
            shrink += footer->get_height_constraint();

        if (posts_box)
        {
            posts_box->set_size(1, term_h() - shrink);
            posts_box->rebuild(false);
        }

        if (main_vbox)
            main_vbox->rebuild(false);

        if (scroll_panel)
            scroll_panel->rebuild(false);

        update_size(true);
        WIDGET_MAN.draw_widgets();
        b_handled = true;
    }
    // enable or disable auto-update
    else if (input_event.key == TB_KEY_CTRL_A)
    {
        b_auto_update = !b_auto_update;

        if (footer && footer_info)
        {
            footer_info->clear_text();
            footer_info->append_text(base_footer_text);

            if (b_auto_update)
            {
                std::ostringstream out;
                uint32_t time = auto_refresh_counter.count();
                time /= 1000;
                time = (auto_refresh_interval.count() / 1000) - time;
                footer_countdown = L"| Reload in ";
                footer_countdown += std::to_wstring(time);
                footer_countdown += L"s";
                footer_info->append_text(footer_countdown, true);
            }
            else
            {
                auto_refresh_counter = std::chrono::milliseconds(0);
                footer_info->append_text("| Reload [paused]", true);
            }

            WIDGET_MAN.draw_widgets(
                footer.get(),
                false /* clear cells */,
                false /* clear images */);
        }

        b_handled = true;
    }
    // close thread
    else if (input_event.key == TB_KEY_CTRL_X)
    {
        THREAD_MAN.kill_jobs(get_id());
        delete_self();
        return true;
    }

    if (!b_handled)
    {
        if (scroll_panel)
        {
            b_handled = scroll_panel->handle_key_input(input_event, false);
        }
    }

    return b_handled;
}



void Thread4chanWidget::save_to_disk() const
{
    if (!b_can_save)
        return;

    std::string save_data = thread_url;
    save_data += "\n";
    save_data += board;
    save_data += "\n";
    save_data += thread_num_str;
    save_data += "\n";
    // thread subject goes last because sometimes it is empty
    save_data += thread_subject;
    save_data += "\n\0";
    std::string thread_dir = get_file_save_dir(thread_url);
    FileOps::write_file(
        thread_dir, SAVE_FILE, save_data.c_str(), save_data.size());
}


void Thread4chanWidget::delete_save_file() const
{
    std::string thread_dir = get_file_save_dir(thread_url);
    FileOps::delete_file(thread_dir + SAVE_FILE);
}


bool Thread4chanWidget::is_saved() const
{
    std::string thread_dir = get_file_save_dir(thread_url);
    return FileOps::file_exists(thread_dir + SAVE_FILE);
}


void Thread4chanWidget::update_header_info()
{
    if (header_info)
    {
        header_info->clear_text();
        header_info->append_text(base_header_text);

        if (b_archived)
            header_info->append_text(
                "[Archived]",
                false,
                false,
                false,
                false,
                COLO.thread_header_bg,
                COLO.thread_archived
            );

        if (is_saved())
            header_info->append_text(
                "[Saved]",
                false,
                false,
                false,
                false,
                COLO.thread_header_bg,
                COLO.thread_saved
            );
    }
}


void Thread4chanWidget::child_widget_size_change_event()
{
}


void Thread4chanWidget::draw_children(vector4d constraint) const
{
    if (child_widget)
    {
        child_widget->draw();
    }
}


vector2d Thread4chanWidget::get_child_widget_size() const
{
    if (child_widget)
    {
        return child_widget->get_size();
    }

    return vector2d();
}


void Thread4chanWidget::update_child_size(bool b_recursive)
{
    if (child_widget)
    {
        child_widget->update_size(b_recursive);
    }
}


void Thread4chanWidget::scroll_to_post(int post_num)
{
    auto post = get_post(post_num);
    if (post && posts_box && scroll_panel)
    {
        vector2d scroll_pos = scroll_panel->get_scroll_position();
        vector2d abs_off = post->get_absolute_offset();
        vector2d posts_off = posts_box->get_inherited_offset();
        scroll_panel->scroll_to(-(abs_off.y - scroll_pos.y - posts_off.y));
        WIDGET_MAN.draw_widgets();
    }
}


void Thread4chanWidget::select_post(Post4chanWidget* post)
{
    if (!post) return;

    if (post != selected_post)
    {
        if (selected_post) selected_post->unselect();
        post->select();
        selected_post = post;
    }
    else
    {
        post->unselect();
        selected_post = nullptr;
    }
}

