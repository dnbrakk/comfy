/**
 * Comfy
 *
 *  Copyright 2019 by Wolfish <wolfish@airmail.cc>
 *  https://wolfish.neocities.org/soft/comfy/
 *
 *  Licensed under the GPL v2.0 only.
 */
#include "replywidget.h"
#include "widgets.h"
#include "../widgetman.h"


ReplyWidget::ReplyWidget(vector2d _offset, vector4d _padding, vector2d _size, uint32_t _bg_color, uint32_t _fg_color, bool _b_fullscreen)
: TermWidget(_offset, _padding, vector4d(), _bg_color, _fg_color, _b_fullscreen)
{
    bg_color = COLO.new_post_bg;
    fg_color = COLO.new_post_fg;

    main_box = nullptr;
    editable_text = nullptr;

    set_h_sizing(e_widget_sizing::ws_fullscreen);
    set_v_sizing(e_widget_sizing::ws_fullscreen);
    set_draw_cursor(true);

    rebuild();
}


void ReplyWidget::rebuild(bool b_rebuild_children)
{
    update_size(false /* recursive */);

    if (!editable_text)
    {
        editable_text = std::make_shared<EditableTextWidget>(
            vector2d(),
            vector4d(1, 0, 1, 0),
            COLO.new_post_bg,
            COLO.new_post_fg);
    }

    scroll_panel = std::make_shared<ScrollPanelWidget>(
        false,
        vector2d(),
        true,
        false,
        COLO.title_bg,
        COLO.title_fg);
    scroll_panel->set_child_widget(editable_text, false);
    scroll_panel->set_h_sizing(e_widget_sizing::ws_fill);
    scroll_panel->set_v_sizing(e_widget_sizing::ws_auto);

    main_box = std::make_shared<BoxWidget>(
        vector2d(),
        vector4d(),
        vector2d(),
        COLO.new_post_bg,
        COLO.new_post_fg);
    main_box->set_h_sizing(ws_fill);
    main_box->set_v_sizing(ws_fill);
    main_box->set_h_align(wa_center);
    main_box->set_v_align(wa_center);
    main_box->set_child_widget(scroll_panel);

    set_child_widget(main_box, false);
    main_box->rebuild(true);

    // scroll if necessary
    update_cursor_coord();

    // constraint:
    //     a = x start
    //     b = x end
    //     c = y start
    //     d = y end
    vector4d constraint = scroll_panel->get_draw_constraint();
    vector2d offs = cursor_coord;

    if (offs.y + scroll_offset.y + 1 > constraint.d)
    {
        scroll_offset.y = -(offs.y - constraint.d + 1);
        scroll_panel->set_scroll_position(scroll_offset);
    }
    else if (offs.y + scroll_offset.y < constraint.c)
    {
        scroll_offset.y = -(offs.y - constraint.c);
        scroll_panel->set_scroll_position(scroll_offset);
    }
    else
    {
        scroll_panel->set_scroll_position(scroll_offset);
    }
}


void ReplyWidget::child_widget_size_change_event()
{
    rebuild(false);
}


vector4d ReplyWidget::get_child_widget_padding() const
{
    if (child_widget)
    {
        return child_widget->get_padding();
    }

    return vector4d();
}


vector2d ReplyWidget::get_child_widget_size() const
{
    if (child_widget)
    {
        return child_widget->get_size();
    }

    return vector2d();
}


void ReplyWidget::update_child_size(bool b_recursive)
{
    if (child_widget)
    {
        child_widget->update_size(b_recursive);
    }
}


void ReplyWidget::update_cursor_coord()
{
    if (!editable_text)
        return;

    editable_text->update_cursor_coord();
    cursor_coord = editable_text->get_cursor_coord();
}


bool ReplyWidget::handle_key_input(const tb_event& input_event, bool b_bubble_up)
{
    bool b_handled = false;

    if (!main_box || !editable_text)
        return b_handled;

    if (input_event.ch != 0)
    {
        editable_text->insert_char(wchar_t(input_event.ch), false);
        rebuild();
        WIDGET_MAN.draw_widgets();
        b_handled = true;
    }
    else if (input_event.key == TB_KEY_SPACE)
    {
        editable_text->insert_char(L' ', false);
        rebuild();
        WIDGET_MAN.draw_widgets();
        b_handled = true;
    }
    else if (input_event.key == TB_KEY_ENTER)
    {
        editable_text->insert_char(L'\n', false);
        rebuild();
        WIDGET_MAN.draw_widgets();
        b_handled = true;
    }
    else if (input_event.key == TB_KEY_BACKSPACE ||
             input_event.key == TB_KEY_BACKSPACE2 ||
             input_event.key == TB_KEY_CTRL_8 ||
             input_event.key == TB_KEY_CTRL_H)
    {
        editable_text->remove_char(false);
        rebuild();
        WIDGET_MAN.draw_widgets();
        b_handled = true;
    }
    else if (input_event.key == TB_KEY_ARROW_LEFT)
    {
        editable_text->move_cursor_left();
        rebuild();
        WIDGET_MAN.draw_widgets();
        b_handled = true;
    }
    else if (input_event.key == TB_KEY_ARROW_RIGHT)
    {
        editable_text->move_cursor_right();
        rebuild();
        WIDGET_MAN.draw_widgets();
        b_handled = true;
    }
    else if (input_event.key == TB_KEY_ARROW_UP)
    {
        editable_text->move_cursor_up();
        rebuild();
        WIDGET_MAN.draw_widgets();
        b_handled = true;
    }
    else if (input_event.key == TB_KEY_ARROW_DOWN)
    {
        editable_text->move_cursor_down();
        rebuild();
        WIDGET_MAN.draw_widgets();
        b_handled = true;
    }
    else if (input_event.key == TB_KEY_CTRL_P)
    {
        submit_reply();
        // TODO: probably shouldn't actually delete the widget yet
        //       in case there was an error submitting the reply.
        //       could just refocus the thread and keep this
        //       widget alive until a response is received
        //       indicating posting success or failure.
        delete_self();
        return true;
    }
    else if (input_event.key == TB_KEY_CTRL_X)
    {
        THREAD_MAN.kill_jobs(get_id());
        delete_self();
        return true;
    }

    return b_handled;
}


void ReplyWidget::receive_left_click(vector2d coord, TermWidget* clicked, TermWidget* source)
{
    if (!editable_text)
        return;
    
    //int index = editable_text->get_index_at_coord(coord);
    //cursor_pos = index;
    //WIDGET_MAN.draw_widgets();
}


const std::wstring& ReplyWidget::get_buffer() const
{
    static std::wstring empty_buf;
    if (!editable_text)
        return empty_buf;

    return editable_text->get_buffer();
}


void ReplyWidget::submit_reply()
{
}

