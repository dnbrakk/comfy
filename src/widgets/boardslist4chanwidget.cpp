/**
 * Comfy
 *
 *  Copyright 2019 by Wolfish <wolfish@airmail.cc>
 *  https://wolfish.neocities.org/soft/comfy/
 *
 *  Licensed under the GPL v2.0 only.
 */
#include "boardslist4chanwidget.h"
#include "../netops.h"
#include "../fileops.h"
#include "../widgetman.h"
#include "widgets.h"


BoardsList4chanWidget::BoardsList4chanWidget(data_4chan& chan_data)
: ChanWidget(vector2d(), vector4d(), vector4d(), COLO.post_bg, COLO.post_text, false)
{
    set_id(chan_data.parser.url);
    set_can_switch_to(false);
    set_h_sizing(e_widget_sizing::ws_fill);
    set_v_sizing(e_widget_sizing::ws_fill);
    b_ticks = false;

    board_selections = nullptr;

    update(chan_data);
}


void BoardsList4chanWidget::handle_term_resize_event()
{
    refresh();
}


void BoardsList4chanWidget::open_board(std::string url)
{
    NetOps::http_get__4chan_json(url, "", true, 0);
}


void BoardsList4chanWidget::refresh(bool b_draw_term)
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


void BoardsList4chanWidget::rebuild(bool b_rebuild_children)
{
    if (!page_data) return;

    int set = 0;
    if (board_selections)
    {
        set = board_selections->get_selection();
    }

    board_selections = std::make_shared<SelectionWidget>();
    board_selections->set_draw_border(false);

    std::string sel;
    std::string board_url;
    for (auto& b : page_data->board_listings)
    {
        sel = "/" + b.path + "/";
        // pad with spaces to make alignment even
        int len = sel.length();
        for (int i = 0; i < 8 - len; ++i)
        {
            sel += " ";
        }
        sel += b.title;
        board_url = "a.4cdn.org/" + b.path + "/catalog.json";
        board_selections->add_selection(
            sel, std::bind(open_board, board_url));
    }

    board_selections->set_selection(set);
    set_child_widget(board_selections, false /* rebuild */);
    board_selections->rebuild(true /* recursive */);

    // sets term size cache
    update_size(false);
}


void BoardsList4chanWidget::on_focus_received()
{
    // switch focus to parent widget that is managed
    // by Widget Manager, if applicable
    TermWidget* parent = parent_widget;
    while (parent)
    {
        std::shared_ptr<TermWidget> p = WIDGET_MAN.get_widget(parent->get_id());
        if (p)
        {
            WIDGET_MAN.focus_widget(p);
            show();
            return;
        }
        else
        {
            parent = parent->get_parent_widget();
        }
    }

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


bool BoardsList4chanWidget::handle_key_input(const tb_event& input_event, bool b_bubble_up)
{
    bool b_handled = false;

    if (board_selections)
    {
        b_handled = board_selections->handle_key_input(input_event, false);
    }

    return b_handled;
}


void BoardsList4chanWidget::child_widget_size_change_event()
{
}


void BoardsList4chanWidget::draw_children(vector4d constraint) const
{
    if (child_widget)
    {
        child_widget->draw();
    }
}


vector2d BoardsList4chanWidget::get_child_widget_size() const
{
    if (child_widget)
    {
        return child_widget->get_size();
    }

    return vector2d();
}


void BoardsList4chanWidget::update_child_size(bool b_recursive)
{
    if (child_widget)
    {
        child_widget->update_size(b_recursive);
    }
}

