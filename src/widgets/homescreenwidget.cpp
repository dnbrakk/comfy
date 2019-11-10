/**
 * Comfy
 *
 *  Copyright 2019 by Wolfish <wolfish@airmail.cc>
 *  https://wolfish.neocities.org/soft/comfy/
 *
 *  Licensed under the GPL v2.0 only.
 */
#include "homescreenwidget.h"
#include "widgets.h"
#include "../widgetman.h"
#include "../fileops.h"
#include "../netops.h"


HomescreenWidget::HomescreenWidget(vector2d _offset, vector4d _padding, vector2d _size, uint32_t _bg_color, uint32_t _fg_color, bool _b_fullscreen)
: ChanWidget(_offset, _padding, vector4d(), _bg_color, _fg_color, true)
{
    set_id("HOMESCREEN");

    set_can_switch_to(false);
    set_draw_img_buffer(false);
    set_remove_img_artifacts_on_draw(true);
    set_take_draw_control(true);

    set_h_sizing(ws_fullscreen);
    set_v_sizing(ws_fullscreen);

    b_ticks = true;
    set_tick_rate(1000 / 60);

    main_box = nullptr;
    vbox = nullptr;
    logo_text = nullptr;
    und_text = nullptr;
    boards_list = nullptr;

    anim_index = 0;
    anim_pass = 0;
}


void HomescreenWidget::tick_event(std::chrono::milliseconds delta)
{
    if (!b_anim_finished && logo_text && und_text)
    {
        std::shared_ptr<TextWidget> wgt = nullptr;
        if (anim_pass < 2)
        {
            wgt = logo_text;
        }
        else
        {
            wgt = und_text;
        }

        // trail colors
        uint32_t c1 = 159;
        uint32_t c2 = 195;
        uint32_t c3 = 221;
        uint32_t c4 = 213;
        uint32_t c5 = 201;

        // final colors
        uint32_t bg = COLO.title_fg;
        uint32_t fg = COLO.title_bg;

        if (anim_pass == 1 || anim_pass == 3)
        {
            c1 = 27;
            c2 = 33;
            c3 = 39;
            c4 = 45;
            c5 = 51;

            bg = COLO.title_bg;
            if (anim_pass == 3)
            {
                fg = COLO.motd;
            }
            else
            {
                fg = COLO.title_fg;
            }
        }

        std::map<int, uint32_t> c_map;
        c_map[0] = c1;
        c_map[1] = c2;
        c_map[2] = c3;
        c_map[3] = c4;
        c_map[4] = c5;

        // color trail
        for (int i = 0; i < 5; ++i)
        {
            wgt->override_format(
                anim_index + i,
                c_map[i],
                c_map[i],
                false,
                false,
                false);
        }

        // final color
        for (int j = 1; j < 4; ++j)
        {
            wgt->override_format(
                anim_index - j,
                bg,
                fg,
                false,
                false,
                false);
        }

        anim_index += 3;
        int index = wgt->get_cell_index();
        if (anim_index > index)
        {
            anim_index = 0;
            anim_pass++;
            if (anim_pass == 4)
            {
                b_anim_finished = true;
            }
        }

        logo_text->rebuild();
        und_text->rebuild();
        WIDGET_MAN.draw_widgets(logo_text.get(), false, false);
        WIDGET_MAN.draw_widgets(und_text.get(), false, false);
    }
}


bool HomescreenWidget::update(data_4chan& chan_data)
{
    if (boards_list && chan_data.parser.pagetype == pt_boards_list)
    {
        return boards_list->update(chan_data);
    }

    return false;
}


void HomescreenWidget::show_4chan_boards_list(HomescreenWidget* hs)
{
    if (!hs || !hs->get_content_box()) return;

    std::string url = "a.4cdn.org/boards.json";
    if (!hs->boards_list)
    {
        data_4chan chan_data(url);
        hs->boards_list = std::make_shared<BoardsList4chanWidget>(chan_data);
    }

    hs->get_content_box()->set_child_widget(hs->boards_list, false);
    hs->get_content_box()->rebuild(true);
    WIDGET_MAN.draw_widgets(hs);

    NetOps::http_get__4chan_json(url, hs->get_id());
}


void HomescreenWidget::show_saved_threads(HomescreenWidget* hs)
{
    if (!hs) return;

    std::shared_ptr<SelectionWidget> wg =
        std::make_shared<SelectionWidget>();
    wg->set_id("SAVED_THREADS");
    wg->set_draw_border(false);
    wg->set_v_align(wa_top);

    std::vector<std::string> dirs = FileOps::get_all_dirs_containing_file_name(DATA_DIR, SAVE_FILE);
    for (auto& d : dirs)
    {
        // SAVE_FILE format for 4chan threads is defined
        // in Thread4chanWidget HandleInput() and is as follows:
        //
        // [thread_url, board, thread_num, thread_subject]
        //
        // each variable is separated by a newline char.
        // thread_subject is sometimes empty
        //
        std::vector<std::string> lines =
            FileOps::get_lines_in_file(d + "/" + SAVE_FILE);
        if (lines.size() > 2)
        {
            std::string item = "4chan /" + lines[1] + "/" + lines[2];
            if (lines.size() > 3)
            {
                item += " - " + lines[3];
            }
            else
            {
                item += " - [empty subject]";
            }

            wg->add_selection(item, std::bind(WIDGET_MAN.open_thread, lines[0], true));
        }
    }

    if (wg->get_num_selections() > 0 && hs->get_content_box())
    {
        hs->get_content_box()->set_child_widget(wg);
        hs->get_content_box()->rebuild(true);
        WIDGET_MAN.draw_widgets(hs);
    }
}


void HomescreenWidget::show_about_info(HomescreenWidget* hs)
{
    if (!hs || !hs->get_content_box()) return;

    std::string about = "Comfy ";
    about += VERSION;
    about += "<br><br>Copyright 2019 by Wolfish <wolfish@airmail.cc>";
    about += "<br>https://wolfish.neocities.org/soft/comfy/";

    // quotes
    about += "<br><br>\"...I would write programs all night, and around seven in the morning I'd go to sleep... What they had in common was mainly love of excellence in programming. They wanted to make their programs that they use be as good as they could. They also wanted to make them do neat things, they wanted to be able to do something in a more exciting way than anyone believed possible, and show 'look how wonderful this is! I bet you didn't believe this could be done.'\"<br> - Richard Stallman";
    about += "<br><br>\"The FSF is full of crazy, bigoted people. That's just my opinion.\"<br> - Linus Torvalds";
    about += "<br><br>\"They got rid of CD/DVD. They are coming for our guns.\"<br> - Terry A. Davis";

    auto about_text = std::make_shared<TextWidget>(
        vector2d(),
        vector4d(),
        COLO.title_bg,
        COLO.title_fg);
    about_text->set_h_align(e_widget_align::wa_center);
    about_text->append_text(about, false);

    auto scroll_panel = std::make_shared<ScrollPanelWidget>(
        false,
        vector2d(),
        true,
        false,
        COLO.title_bg,
        COLO.title_fg);
    scroll_panel->set_child_widget(about_text, false);
    scroll_panel->set_h_sizing(e_widget_sizing::ws_fill);
    scroll_panel->set_v_sizing(e_widget_sizing::ws_auto);

    hs->get_content_box()->set_child_widget(scroll_panel);
    hs->get_content_box()->rebuild(true);
    WIDGET_MAN.draw_widgets(hs);
}


void HomescreenWidget::show_help(HomescreenWidget* hs)
{
    if (!hs || !hs->get_content_box()) return;

    std::string help;
    help += "Controls:\n\n";
    help += "Backspace/CTRL+H         Go to homescreen, or back one level in list\n";
    help += "CTRL+Q                   Quit\n";
    help += "CTRL+X                   Close catalog or thread\n";
    help += "CTRL+R                   Reload the focused page from the network\n";
    help += "CTRL+A                   Enable/disable auto-reload\n";
    help += "CTRL+S                   Save currently focused thread\n";
    help += "F5                       Do a hard refresh of the screen\n";
    help += "Tab                      Switch between currently opened pages\n";
    help += "Space/Enter              Choose selection in list\n";
    help += "Up/Down Arrow Keys       Scroll up/down one line at a time\n";
    help += "Left/Right Arrow Keys    Scroll up/down one screen at a time\n";
    help += "Page Up/Down             Scroll up/down one screen at a time\n";
    help += "Home                     Go to top of page\n";
    help += "End                      Go to bottom of page\n";
    help += "Mouse Wheel              Scroll the currently focused page\n";
    help += "Left-Click               Open a thread in a catalog,\n";
    help += "                         full screen/close an image in a thread,\n";
    help += "                         or go to a post in a thread\n";
    help += "                         (by clicking a post number link)\n";

    auto help_text = std::make_shared<TextWidget>(
        vector2d(),
        vector4d(),
        COLO.title_bg,
        COLO.title_fg);
    help_text->set_h_align(e_widget_align::wa_center);
    help_text->append_raw_text(help, false);

    auto scroll_panel = std::make_shared<ScrollPanelWidget>(
        false,
        vector2d(),
        true,
        false,
        COLO.title_bg,
        COLO.title_fg);
    scroll_panel->set_child_widget(help_text, false);
    scroll_panel->set_h_sizing(e_widget_sizing::ws_fill);
    scroll_panel->set_v_sizing(e_widget_sizing::ws_auto);

    hs->get_content_box()->set_child_widget(scroll_panel);
    hs->get_content_box()->rebuild(true);
    WIDGET_MAN.draw_widgets(hs);
}


void HomescreenWidget::quit_application()
{
    WIDGET_MAN.stop();
}


void HomescreenWidget::rebuild(bool b_rebuild_children)
{
    update_size(false /* recursive */);

    vbox = std::make_shared<VerticalBoxWidget>();
    vbox->set_h_align(e_widget_align::wa_center);

    if (!logo_text)
    {
        std::string logo;
        logo += "    __                             __              __\n";
        logo += "   / /  ___    ___    _ __ ___    / _|  _   _     / /\n";
        logo += "  / /  / __|  / _ \\  | '_ ` _ \\  | |_  | | | |   / / \n";
        logo += " / /  | (__  | (_) | | | | | | | |  _| | |_| |  / /  \n";
        logo += "/_/    \\___|  \\___/  |_| |_| |_| |_|    \\__, | /_/   \n";
        logo += "                                        |___/        \n";
                                                              
        logo_text = std::make_shared<TextWidget>(
            vector2d(),
            vector4d(),
            COLO.title_bg,
            COLO.title_bg);
        // force text widget to width of text itself (auto size)
        logo_text->set_h_sizing(e_widget_sizing::ws_auto);
        logo_text->set_h_align(e_widget_align::wa_center);
        logo_text->append_raw_text(logo, true);

        std::wstring motd = L" Live the /comfy/ posting experience ";
        std::wstring und;
        for (int i = 0; i < 3; ++i)
        {
            und += L"\u2500";
        }
        und += motd;
        for (int i = 0; i < logo_text->get_size().x - 1 - motd.length() - 3; ++i)
        {
            und += L"\u2500";
        }
        und += L" ";    // match trailing space on logo text lines

        und_text = std::make_shared<TextWidget>(
            vector2d(),
            vector4d(),
            COLO.title_bg,
            COLO.title_bg);
        // force text widget to width of text itself (auto size)
        und_text->set_h_sizing(e_widget_sizing::ws_auto);
        und_text->set_h_align(e_widget_align::wa_center);
        und_text->append_raw_text(und, true);
    }

    if (!main_sel || !content_box)
    {
        main_sel = std::make_shared<SelectionWidget>();
        main_sel->set_draw_border(false);
        main_sel->set_v_align(wa_top);
        main_sel->add_selection(
            "Browse 4chan", std::bind(show_4chan_boards_list, this));
        main_sel->add_selection(
            "Browse Saved Threads", std::bind(show_saved_threads, this));
        main_sel->add_selection(
            "About", std::bind(show_about_info, this));
        main_sel->add_selection(
            "Help", std::bind(show_help, this));
        main_sel->add_selection("Quit", std::bind(quit_application));

        content_box = std::make_shared<BoxWidget>(
            vector2d(),
            vector4d(0, 1, 0, 0),
            vector2d(),
            COLO.post_bg,
            COLO.post_border);
        content_box->set_h_sizing(e_widget_sizing::ws_fill);
        content_box->set_v_sizing(e_widget_sizing::ws_fill);
        content_box->set_child_padding(vector4d());
        content_box->set_draw_border(false);
        content_box->set_child_widget(main_sel);
    }

    vbox->add_child_widget(logo_text, false);
    vbox->add_child_widget(und_text, false);
    vbox->add_child_widget(content_box, false);

    main_box = std::make_shared<BoxWidget>(
        vector2d(),
        vector4d(),
        vector2d(),
        COLO.post_bg,
        COLO.post_border);
    main_box->set_h_sizing(e_widget_sizing::ws_fill);
    main_box->set_v_sizing(e_widget_sizing::ws_fill);
    main_box->set_draw_border(false);
    main_box->set_child_widget(vbox);
    vector4d pad = main_box->get_child_padding();
    // force min size of box to be size of logo text
    main_box->set_min_width(logo_text->get_size().x - 1 + pad.a + pad.c);

    set_child_widget(main_box, false);
    main_box->rebuild(true);
}


bool HomescreenWidget::handle_key_input(const tb_event& input_event, bool b_bubble_up)
{
    bool b_handled = false;

    if (input_event.key == TB_KEY_BACKSPACE ||
        input_event.key == TB_KEY_BACKSPACE2 ||
        input_event.key == TB_KEY_CTRL_8 ||
        input_event.key == TB_KEY_CTRL_H)
    {
        if (content_box)
        {
            content_box->set_child_widget(main_sel);
            content_box->rebuild(true);
            WIDGET_MAN.draw_widgets();
        }

        b_handled = true;
    }
    else if (input_event.key == TB_KEY_TAB)
    {
        WIDGET_MAN.open_switch_widget();
        b_handled = true;
    }
    else if (input_event.key == TB_KEY_CTRL_R)
    {
        logo_text = nullptr;
        und_text = nullptr;
        b_anim_finished = false;
        anim_index = 0;
        anim_pass = 0;
        rebuild();
        b_handled = true;
    }
    else if (content_box && content_box->get_child_widget())
    {
        b_handled =
            content_box->get_child_widget()->handle_key_input(input_event, false);
    }

    return b_handled;
}


void HomescreenWidget::handle_term_resize_event()
{
    rebuild();
}


void HomescreenWidget::child_widget_size_change_event()
{
}


vector4d HomescreenWidget::get_child_widget_padding() const
{
    if (child_widget)
    {
        return child_widget->get_padding();
    }

    return vector4d();
}


vector2d HomescreenWidget::get_child_widget_size() const
{
    if (child_widget)
    {
        return child_widget->get_size();
    }

    return vector2d();
}


void HomescreenWidget::update_child_size(bool b_recursive)
{
    if (child_widget)
    {
        child_widget->update_size(b_recursive);
    }
}

