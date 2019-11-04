/**
 * Comfy
 *
 *  Copyright 2019 by Wolfish <wolfish@airmail.cc>
 *  https://wolfish.neocities.org/soft/comfy/
 *
 *  Licensed under the GPL v2.0 only.
 */
#include "scrollpanelwidget.h"
#include "../widgetman.h"
#include "scrollbarwidget.h"


ScrollPanelWidget::ScrollPanelWidget(bool _b_has_scrollbar, vector2d _offset, bool _b_allow_vert_scroll, bool _b_allow_horiz_scroll, uint32_t _bg_color, uint32_t _fg_color)
: TermWidget(_offset, vector4d(), vector4d(), _bg_color, _fg_color)
, scroll_bar_widget(nullptr)
, b_has_scrollbar(_b_has_scrollbar)
, b_allow_vert_scroll(_b_allow_vert_scroll)
, b_allow_horiz_scroll(_b_allow_horiz_scroll)
{
    if (b_has_scrollbar)
    {
        // make room for scroolbar on screen
        child_padding = vector4d(0, 0, 1, 0);
    }

    rebuild();
}


bool ScrollPanelWidget::handle_key_input(const tb_event& input_event, bool b_bubble_up)
{
    bool b_handled = false;
    vector4d pad = get_padding();

    if (b_allow_vert_scroll)
    {
        switch(input_event.key)
        {
            case TB_KEY_ARROW_DOWN          :   scroll(-1);
                                                b_handled = true;
                                                break;

            case TB_KEY_MOUSE_WHEEL_DOWN    :   scroll(-1);
                                                b_handled = true;
                                                break;

            case TB_KEY_ARROW_UP            :   scroll(1);
                                                b_handled = true;
                                                break;

            case TB_KEY_MOUSE_WHEEL_UP      :   scroll(1);
                                                b_handled = true;
                                                break;

            case TB_KEY_PGUP                :   scroll(get_visible_height());
                                                b_handled = true;
                                                break;

            case TB_KEY_ARROW_LEFT          :   scroll(get_visible_height());
                                                b_handled = true;
                                                break;

            case TB_KEY_PGDN                :   scroll(-get_visible_height());
                                                b_handled = true;
                                                break;

            case TB_KEY_ARROW_RIGHT         :   scroll(-get_visible_height());
                                                b_handled = true;
                                                break;

            case TB_KEY_HOME                :   scroll_to_beginning();
                                                b_handled = true;
                                                break;

            case TB_KEY_END                 :   scroll_to_end();
                                                b_handled = true;
                                                break;
        }
    }
    else if (b_allow_horiz_scroll && input_event.key == TB_KEY_ARROW_LEFT)
    {
        if (offset.x > 0 || size.x + offset.x + pad.a + pad.c > term_w())
        {
            offset.x--;
        }

        b_handled = true;
    }
    else if (b_allow_horiz_scroll && input_event.key == TB_KEY_ARROW_RIGHT)
    {
        if (offset.x < 0)
        {
            offset.x++;
        }

        b_handled = true;
    }

    if (b_handled)
    {
        // redraw
        WIDGET_MAN.draw_widgets();
    }

    return b_handled;
}


void ScrollPanelWidget::update_scroll_bar()
{
    if (b_has_scrollbar && scroll_bar_widget)
    {
        scroll_bar_widget->rebuild();
    }
}


void ScrollPanelWidget::scroll(int dist)
{
    offset.y += dist;
    scroll_pos_sanity_check();
    update_scroll_bar();
}


void ScrollPanelWidget::scroll_to(int pos)
{
    offset.y = pos;
    scroll_pos_sanity_check();
    update_scroll_bar();
}


void ScrollPanelWidget::scroll_to_end()
{
    vector4d pad = get_padding();
    offset.y = -(size.y - get_visible_height() + pad.b + pad.d);
    update_scroll_bar();
}


void ScrollPanelWidget::scroll_to_beginning()
{
    offset.y = 0;
    update_scroll_bar();
}


int ScrollPanelWidget::get_visible_height()
{
    int vis = 0;

    TermWidget* wgt = parent_widget;
    while(wgt && wgt->get_v_sizing() == e_widget_sizing::ws_auto)
    {
        wgt = wgt->get_parent_widget();
    }

    if (wgt)
    {
        vis = wgt->get_height_constraint();

        vector2d off = wgt->get_absolute_offset();
        // if offset is pushing some of the widget off screen
        if (off.y > 0 && off.y + vis > term_h())
        {
            vis -= off.y;
        }

        // padding is handled in draw_children()
        //vector4d pad = get_padding();
        //vis -= pad.b + pad.d;
    }
    else
    {
        vis = term_h();
    }

    return vis;
}


int ScrollPanelWidget::get_visible_width()
{
    int vis = 0;

    TermWidget* wgt = parent_widget;
    while(wgt && wgt->get_h_sizing() == e_widget_sizing::ws_auto)
    {
        wgt = wgt->get_parent_widget();
    }

    if (wgt)
    {
        vis = wgt->get_width_constraint();
    }
    else
    {
        vis = term_w();
    }

    vector2d off = get_inherited_offset();
    // if offset is pushing some of the widget off screen
    if (off.x > 0 && off.x + vis > term_w())
    {
        vis -= off.x;
    }

    // padding is handled in draw_children()
    //vector4d pad = get_padding();
    //vis -= pad.a + pad.c;

    return vis;
}


void ScrollPanelWidget::scroll_pos_sanity_check()
{
    vector4d pad = get_padding();
    int vis_h = get_visible_height();
    vector2d c_size = get_child_widget_size();
    // if child smaller than visible space
    if (c_size.y <= vis_h)
    {
        offset.y = 0;
    }
    // if over-scrolled
    else if (offset.y > 0)
    {
        offset.y = 0;
    }
    // prevent scroll past end
    else if (offset.y < -(size.y - get_visible_height() + pad.b + pad.d))
    {
        offset.y = -(size.y - get_visible_height() + pad.b + pad.d);
    }
}


void ScrollPanelWidget::rebuild(bool b_rebuild_children)
{
    if (child_widget)
    {
        if (b_rebuild_children)
        {
            child_widget->rebuild();
        }

        update_size(false /* recursive */);
    }

    scroll_pos_sanity_check();

    if (b_has_scrollbar)
    {
        if (!scroll_bar_widget)
        {
            scroll_bar_widget = std::make_shared<ScrollBarWidget>(this, COLO.scroll_bar_bg, COLO.scroll_bar_fg);
        }

        if (scroll_bar_widget)
        {
            scroll_bar_widget->rebuild();
        }
    }

    cells.clear();
}


void ScrollPanelWidget::child_widget_size_change_event()
{
    rebuild(false);
}


vector2d ScrollPanelWidget::get_child_widget_size() const
{
    if (child_widget)
    {
        return child_widget->get_size();
    }

    return vector2d();
}


void ScrollPanelWidget::update_child_size(bool b_recursive)
{
    if (child_widget)
    {
        child_widget->update_size();
        if (b_recursive)
        {
            child_widget->update_child_size(true);
        }
    }
}


vector4d ScrollPanelWidget::get_draw_constraint()
{
    int vis_w = get_visible_width();
    int vis_h = get_visible_height();
    vector4d pad = get_padding();
    vector2d off = get_inherited_offset();
    if (parent_widget)
    {
        off = parent_widget->get_absolute_offset();
    }

    return vector4d(
        off.x + pad.a,          // x start
        vis_w + off.x - pad.c,  // x end
        off.y + pad.b,          // y start
        vis_h + off.y - pad.d); // y end
}


void ScrollPanelWidget::draw_children(vector4d constraint) const
{
    if (child_widget)
    {
        child_widget->draw(get_draw_constraint());
    }

    if (scroll_bar_widget)
    {
        scroll_bar_widget->draw();
    }
}

