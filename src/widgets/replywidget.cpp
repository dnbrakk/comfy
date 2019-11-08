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
    post_text = nullptr;
    cursor_pos = -1;

    set_h_sizing(e_widget_sizing::ws_fullscreen);
    set_v_sizing(e_widget_sizing::ws_fullscreen);
    set_draw_cursor(true);

    rebuild();
}


void ReplyWidget::rebuild(bool b_rebuild_children)
{
    update_size(false /* recursive */);

    post_text = std::make_shared<TextWidget>(
        vector2d(),
        vector4d(1, 0, 1, 0),
        COLO.new_post_bg,
        COLO.new_post_fg);
    post_text->set_h_sizing(e_widget_sizing::ws_fill);
    post_text->set_v_sizing(e_widget_sizing::ws_dynamic);
    post_text->append_text(post_text_buf, false);

    scroll_panel = std::make_shared<ScrollPanelWidget>(
        false,
        vector2d(),
        true,
        false,
        COLO.title_bg,
        COLO.title_fg);
    scroll_panel->set_child_widget(post_text, false);
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
    if (!post_text)
        return;

    cursor_coord = post_text->get_coord_of_index(cursor_pos);
    cursor_coord.x += 1;
}


bool ReplyWidget::handle_key_input(const tb_event& input_event, bool b_bubble_up)
{
    bool b_handled = false;

    if (!main_box || !post_text)
        return b_handled;

    if (input_event.ch != 0)
    {
        post_text_buf += wchar_t(input_event.ch);
        cursor_pos++;
        post_text->append_char(wchar_t(input_event.ch), true, cursor_pos);
        WIDGET_MAN.draw_widgets();

        b_handled = true;
    }
    else if (input_event.key == TB_KEY_SPACE)
    {
        post_text_buf += L" ";
        cursor_pos++;
        post_text->append_char(L' ', true, cursor_pos);
        WIDGET_MAN.draw_widgets();

        b_handled = true;
    }
    else if (input_event.key == TB_KEY_ENTER)
    {
        post_text_buf += L"\n";
        cursor_pos++;
        post_text->append_char(L'\n', true, cursor_pos);
        WIDGET_MAN.draw_widgets();

        b_handled = true;
    }
    else if (input_event.key == TB_KEY_BACKSPACE ||
             input_event.key == TB_KEY_BACKSPACE2 ||
             input_event.key == TB_KEY_CTRL_8 ||
             input_event.key == TB_KEY_CTRL_H)
    {
        if (post_text_buf.size() > 0)
        {
            if (cursor_pos + 1 < post_text_buf.length())
            {
                post_text_buf =
                    post_text_buf.substr(0, cursor_pos) +
                    post_text_buf.substr(cursor_pos + 1);
            }
            else
            {
                post_text_buf = post_text_buf.substr(0, cursor_pos);
            }

            post_text->remove_char(cursor_pos, true);
            cursor_pos--;
            WIDGET_MAN.draw_widgets();
        }

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
    if (!post_text)
        return;
    
    int index = post_text->get_index_at_coord(coord);
    ERR(index, "CLICK INDEX");
    cursor_pos = index;
    WIDGET_MAN.draw_widgets();
}


void ReplyWidget::submit_reply()
{
}

