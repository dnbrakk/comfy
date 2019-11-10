/**
 * Comfy
 *
 *  Copyright 2019 by Wolfish <wolfish@airmail.cc>
 *  https://wolfish.neocities.org/soft/comfy/
 *
 *  Licensed under the GPL v2.0 only.
 */
#include "widgetman.h"

#include "fileops.h"
#include "netops.h"
#include "imgman.h"
#include "widgets/widgets.h"
//#include <thread>


void WidgetMan::init()
{
    if (!b_init)
    {
        tb_init();
        tb_select_input_mode(TB_INPUT_ESC | TB_INPUT_MOUSE);
        tb_select_output_mode(TB_OUTPUT_256);
        tb_set_clear_attributes(COLO.app_bg, COLO.app_bg);
        tb_clear();
        tb_present();

        term_size_cache = vector2d(tb_width(), tb_height());
        // dependent on term_size_cache
        clear_termbox_frame(true /* b_resize */);

        set_draw_img_buffer(true);
        draw_controller = nullptr;

        homescreen = std::make_shared<HomescreenWidget>();
        homescreen->rebuild();
        add_widget(homescreen, true);

        b_init = true;
    }
}


void WidgetMan::switch_to_homescreen()
{
    homescreen->rebuild();
    move_to_front(homescreen, true);
    draw_widgets();
}


void WidgetMan::open_thread(std::string url, bool b_steal_focus)
{
    data_4chan chan_data(url);
    chan_data.b_steal_focus = b_steal_focus;
    std::stringstream buf;
    FileOps::read_file(
        buf,
        chan_data.file_path + "thread.json");
    bool b_parsed = chan_data.parse_json(buf.str().c_str());
    // json exists in cache, load from disk
    if (b_parsed)
    {
        NetOps::queue__4chan_json.push(chan_data);
    }
    // try load url
    else
    {
        NetOps::http_get__4chan_json(
            url,
            "",
            b_steal_focus,
            0 /* last fetch time */,
            url /* job_pool_id */,
            true /* b_push_to_front (of job queue)*/);
    }
}


// widgets cannot be added if widget->get_id() == ""
std::shared_ptr<TermWidget> WidgetMan::add_widget(std::shared_ptr<TermWidget> widget, bool b_focus)
{
    if (!widget || widget->get_id().empty() || get_widget(widget->get_id()))
    {
        return nullptr;
    }

    widget_stack.push_back(widget);

    if (b_focus)
    {
        focus_widget(widget);
    }

    return widget;
}


void WidgetMan::focus_widget(std::shared_ptr<TermWidget> widget)
{
    if (!widget || focused_widget == widget)
    {
        return;
    }

    // relinquish draw privileges so newly focused widget can draw to term
    draw_controller = nullptr;
    // take draw control privileges
    if (widget->should_take_draw_control())
    {
        draw_controller = widget.get();
    }

    // whether or not to draw image buffer when this widget is focused
    b_draw_img_buffer = widget->should_draw_img_buffer();

    auto prev_focused = focused_widget;
    focused_widget = widget;
    if (focused_widget)
    {
        focused_widget->on_focus_received();
    }

    if (prev_focused)
    {
        prev_focused->on_focus_lost();
    }
}


void WidgetMan::run()
{
    if (b_run) return;

    init();
    draw_widgets();

    b_run = true;
    tb_event input_event;
    last_tick_time = time_now_us();

    while (b_run)
    {
        std::chrono::microseconds now = time_now_us();
        std::chrono::microseconds delta = now - last_tick_time;
        last_tick_time = now;

        tick_widgets();

        int e_type = tb_peek_event(&input_event, 10 /* timeout in ms */);
        // key input
        if (e_type == TB_EVENT_KEY || e_type == TB_EVENT_MOUSE)
        {
            if (input_event.key == TB_KEY_MOUSE_LEFT)
            {
                if (focused_widget)
                {
                    TermWidget* clicked_child =
                        focused_widget->get_topmost_child_at(
                            vector2d(input_event.x, input_event.y));

                    if (clicked_child)
                    {
                        clicked_child->receive_left_click(
                            vector2d(input_event.x, input_event.y),
                            clicked_child);
                    }
                }
            }
            else
            {
                if (focused_widget)
                {
                    if (!focused_widget->handle_key_input(input_event))
                    {
                        handle_key_input(input_event);
                    }
                }
                else
                {
                    handle_key_input(input_event);
                }
            }
        }
        // terminal resize event
        else if (e_type == TB_EVENT_RESIZE)
        {
            handle_term_resize_event(input_event);
        }

        // load chan data received from worker threads
        load_chan_data();

        // redraw images
        if (DISPLAY_IMAGES)
        {
            // load image data received from worker threads
            load_image_data();

            if (b_draw_img_buffer)
            {
                IMG_MAN.redraw_buffer(true);
            }
        }
    }

    shutdown();
}


void WidgetMan::tick_widgets()
{
    // TODO: tick other widgets
    if (focused_widget)
    {
        focused_widget->tick();
    }
}


void WidgetMan::load_chan_data()
{
    data_4chan chan_data;
    bool b_popped = NetOps::queue__4chan_json.try_pop(chan_data, std::chrono::milliseconds(10));
    if (b_popped)
    {
        load_4chan_data(chan_data);
    }
}


void WidgetMan::load_4chan_data(data_4chan& chan_data)
{
    std::shared_ptr<TermWidget> wgt = get_widget(chan_data.wgt_id);
    // widget exists, update it
    if (wgt)
    {
        ChanWidget* chan_wgt = dynamic_cast<ChanWidget*>(wgt.get());
        if (chan_wgt)
        {
            bool b_changed = chan_wgt->update(chan_data);

            if (wgt != focused_widget && chan_data.b_steal_focus)
            {
                move_to_front(wgt, true);
            }

            // redraw if changed and widget is focused
            if (b_changed &&
                (wgt == focused_widget ||
                 wgt->is_child_of(focused_widget.get())))
            {
                draw_widgets();
            }
        }
    }
    // create new widget
    // if chan_data.b_steal_focus is false, then the chan_data
    // was intended to update a widget that no longer exists
    else if (chan_data.is_valid() && chan_data.b_steal_focus)
    {
        if (chan_data.parser.pagetype == e_page_type::pt_thread)
        {
            std::shared_ptr<Thread4chanWidget> chan_wgt =
                std::make_shared<Thread4chanWidget>(chan_data);
            add_widget(chan_wgt, true /* b_focus */);
        }
        else if (chan_data.parser.pagetype == e_page_type::pt_boards_list)
        {
            std::shared_ptr<BoardsList4chanWidget> chan_wgt =
                std::make_shared<BoardsList4chanWidget>(chan_data);
            add_widget(chan_wgt, true /* b_focus */);
        }
        else if (chan_data.parser.pagetype == e_page_type::pt_board_catalog)
        {
            std::shared_ptr<Catalog4chanWidget> chan_wgt =
                std::make_shared<Catalog4chanWidget>(chan_data);
            add_widget(chan_wgt, true /* b_focus */);
        }
    }
    // error
    else if (!chan_data.is_valid() && chan_data.error_type != et_not_mod_since)
    {
        std::string err = "WIDGET_MAN ERROR: Page failed to load.\n";
        err += "URL: " + chan_data.parser.url + "\n";

        switch(chan_data.error_type)
        {
            case et_invalid_url         : err += "Invalid URL.\n";
                                          break;
            case et_http_404            : err += "404 not found.\n";
                                          break;
            case et_json_parse          : err += "JSON parse error.\n";
                                          break;
        }

        err += "CURL error code: " + std::to_string(chan_data.curl_result);
        err += "\nHTTP response: " + std::to_string(chan_data.http_response);
        ERR(err);
    }
}


void WidgetMan::load_image_data()
{
    if (!DISPLAY_IMAGES) return;

    bool b_redraw = false;
    std::set<TermWidget*> wgts;

    // loaded from disk
    img_packet pac;
    while(IMG_MAN.queue__image_packet.try_pop(pac, std::chrono::milliseconds(10)))
    {
        bool b_loaded = false;

        std::shared_ptr<TermWidget> wgt = get_widget(pac.widget_id);
        if (wgt)
        {
            b_loaded = wgt->receive_img_packet(pac);
            if (b_loaded)
            {
                wgts.insert(wgt.get());
                if (wgt == focused_widget)
                {
                    b_redraw = true;
                }
            }
        }

        // remove image from cache map
        // (happens automatically when checkout_token
        //  in img_packet destructs)
        //if (!b_loaded)
        //{
        //    ERR("Image packet not claimed, removing from cache.");
        //}
    }

    if (b_redraw)
        draw_widgets();
}


std::shared_ptr<TermWidget> WidgetMan::get_widget(std::string key)
{
    for (auto& w : widget_stack)
    {
        if (w && w->get_id().compare(key) == 0)
        {
            return w;
        }
    }

    return nullptr;
}


void WidgetMan::stop()
{
    b_run = false;
}


void WidgetMan::switch_widget(std::shared_ptr<TermWidget> wgt)
{
    if (wgt)
    {
        WIDGET_MAN.move_to_front(wgt, true);
        THREAD_MAN.move_jobs_to_front(wgt->get_id());
    }

    WIDGET_MAN.remove_widget(WIDGET_MAN.switch_widget_select);
    WIDGET_MAN.switch_widget_select = nullptr;
}


void WidgetMan::open_switch_widget()
{
    if (switch_widget_select)
    {
        remove_widget(switch_widget_select);
    }

    switch_widget_select = std::make_shared<SelectionWidget>(
        COLO.switch_widget_bg,
        COLO.switch_widget_fg,
        COLO.switch_widget_border);
    switch_widget_select->set_draw_img_buffer(false);
    switch_widget_select->set_remove_img_artifacts_on_draw(true);
    // take control of drawing privileges, preventing other
    // widgets from calling draw_widgets()
    switch_widget_select->set_take_draw_control(true);
    switch_widget_select->set_id("SWITCH_WIDGET_SELECT");

    std::string sel;
    for (auto it = widget_stack.rbegin(); it != widget_stack.rend(); ++it)
    {
        auto w = *it;
        if (w && w->can_switch_to())
        {
            sel = w->get_title();
            switch_widget_select->add_selection(
                sel, std::bind(switch_widget, w));
        }
    }

    if (switch_widget_select->get_num_selections() > 0)
    {
        switch_widget_select->rebuild(true /* recursive */);
        add_widget(switch_widget_select, true /* b_focus */);
    }
}


void WidgetMan::handle_key_input(const tb_event& input_event)
{
    if (input_event.key == TB_KEY_CTRL_Q)
    {
        stop();
    }
    else if (input_event.key == TB_KEY_F5)
    {
        hard_refresh();
    }
    else if (input_event.key == TB_KEY_TAB)
    {
        open_switch_widget();
    }
    else if (input_event.key == TB_KEY_BACKSPACE ||
             input_event.key == TB_KEY_BACKSPACE2 ||
             input_event.key == TB_KEY_CTRL_8 ||
             input_event.key == TB_KEY_CTRL_H)
    {
        switch_to_homescreen();
    }
}


void WidgetMan::handle_term_resize_event(const tb_event& resize_event)
{
    vector2d term_size(resize_event.w, resize_event.h);
    if (term_size == term_size_cache)
    {
        return;
    }

    term_size_cache = term_size;
    clear_termbox_frame(true /* b_resize */);

    if (focused_widget)
    {
        focused_widget->handle_term_resize_event();
        draw_widgets();
    }
}


void WidgetMan::add_cell_to_frame(term_cell& cell)
{
    int index = cell.coord.x + (cell.coord.y * term_size_cache.x);
    if (index >= 0 && index < termbox_frame.size())
    {
        termbox_frame[index] = cell;
    }
}


void WidgetMan::put_termbox_frame(bool b_clear)
{
    for (auto& cell : termbox_frame)
    {
        if (!cell.is_null())
        {
            tb_put_cell(cell.coord.x, cell.coord.y, &cell.tb_data);
        }
    }

    if (b_clear) clear_termbox_frame(false);
}


void WidgetMan::clear_termbox_frame(bool b_resize)
{
    termbox_frame.clear();

    if (b_resize)
    {
        blank_termbox_frame.clear();
        for (int y = 0; y < term_size_cache.y; ++y)
        {
            for (int x = 0; x < term_size_cache.x; ++x)
            {
                // this constructor initializes
                // with tb_cell.ch == TB_NULL_CHAR
                blank_termbox_frame.emplace_back(vector2d(x, y));
            }
        }
    }

    termbox_frame = blank_termbox_frame;
}


void WidgetMan::remove_image_artifact(vector2d coord)
{
    artifact_remove.push_back(coord);
}


void WidgetMan::draw_widgets(TermWidget* draw_widget, bool b_clear_cells, bool b_clear_images)
{
    // a widget can take control of drawing widgets, preventing
    // other widgets from calling draw_widgets()
    // if draw_conttroller == nullptr, any widget can cann draw_widgets()
    if (draw_controller &&
        draw_widget != draw_controller &&
        focused_widget.get() != draw_controller)
    {
        bool b_parent_is_controller = false;
        if (draw_widget)
        {
            // if widget is child or sub-child of draw_controller,
            // then allow widget to draw
            b_parent_is_controller = draw_widget->is_child_of(draw_controller);
        }

        if (!b_parent_is_controller)
        {
            return;
        }
    }

    // wipe terminal screen, including images
    if (b_clear_cells)
    {
        termbox_clear();
    }

    if (DISPLAY_IMAGES)
    {
        // clear images
        if (b_clear_images)
        {
            IMG_MAN.clear_buffer();
        }
    }

    // termbox_frame and null cells are populated by draw()
    if (draw_widget)
    {
        draw_widget->draw();
    }
    else if (focused_widget)
    {
        focused_widget->draw();
    }
    // nothing to draw
    else
    {
        termbox_draw();
        return;
    }

    // remove image artifacts
    if (artifact_remove.size() > 0)
    {
        std::vector<term_cell> frame_buf = termbox_frame;
        for (auto coord : artifact_remove)
        {
            term_cell cel;
            uint32_t ch;
            tb_utf8_char_to_unicode(&ch, " ");
            cel.tb_data.ch = ch;
            cel.tb_data.bg = COLO.img_artifact_remove;
            cel.tb_data.fg = COLO.img_artifact_remove;
            cel.coord = coord;
            add_cell_to_frame(cel);
        }

        put_termbox_frame();
        apply_null_cells(false /* clear array */);
        termbox_draw();
        termbox_frame = frame_buf;
        artifact_remove.clear();
        //std::this_thread::sleep_for(std::chrono::seconds(3));
    }

    put_termbox_frame();
    apply_null_cells();

    if (DISPLAY_IMAGES)
    {
        if (b_clear_images)
        {
            IMG_MAN.sync();
        }
        else if (b_draw_img_buffer)
        {
            IMG_MAN.redraw_buffer();
        }
    }

    termbox_draw();
}


void WidgetMan::termbox_clear(uint32_t color)
{
    tb_set_clear_attributes(color, color);
    tb_clear();
}


void WidgetMan::termbox_draw()
{
    tb_present();
}


void WidgetMan::hard_refresh()
{
    termbox_clear();
    clear_termbox_frame(true);
    termbox_draw();
    draw_widgets();
}


void WidgetMan::add_null_cell(vector2d cell)
{
    null_cells.push_back(cell);
}


void WidgetMan::apply_null_cells(bool b_clear)
{
    tb_cell nul;
    nul.ch = TB_NULL_CHAR;
    for (auto& cell : null_cells)
    {
        tb_put_cell(cell.x, cell.y, &nul);
    }

    if (b_clear) null_cells.clear();
}


bool WidgetMan::remove_widget(TermWidget* widget)
{
    if (!widget) return false;

    for (auto& w : widget_stack)
    {
        if (w && w.get() == widget)
        {
            return remove_widget(w);
        }
    }

    return false;
}


bool WidgetMan::remove_widget(std::shared_ptr<TermWidget> widget)
{
    if (!widget)
    {
        return false;
    }

    for (int i = widget_stack.size() - 1; i >= 0; --i)
    {
        if (widget_stack[i] == widget)
        {
            widget_stack.erase(widget_stack.begin() + i);
        }
    }

    if (widget == focused_widget)
    {
        focused_widget = nullptr;
        if (!widget_stack.empty())
        {
            focus_widget(widget_stack[widget_stack.size() - 1]);
        }
    }

    return true;
}


void WidgetMan::move_to_front(std::shared_ptr<TermWidget> widget, bool b_focus)
{
    if (widget_stack.back() == widget) return;

    null_cells.clear();
    artifact_remove.clear();

    remove_widget(widget);
    add_widget(widget, b_focus);
}


void WidgetMan::shutdown()
{
    if (b_init)
    {
        widget_stack.clear();
        switch_widget_select = nullptr;
        focused_widget = nullptr;
        homescreen = nullptr;
        draw_controller = nullptr;

        tb_shutdown();
    }
}

