/**
 * Comfy
 *
 *  Copyright 2019 by Wolfish <wolfish@airmail.cc>
 *  https://wolfish.neocities.org/soft/comfy/
 *
 *  Licensed under the GPL v2.0 only.
 */
#include "termwidget.h"
#include "../widgetman.h"


TermWidget::TermWidget(vector2d _offset, vector4d _padding, vector4d _child_padding, uint32_t _bg_color, uint32_t _fg_color, bool _b_fullscreen)
: parent_widget(nullptr)
, child_widget(nullptr)
, offset(_offset)
, default_padding(_padding)
, inherited_padding(vector4d())
, child_padding(_child_padding)
, h_sizing(e_widget_sizing::ws_fill)
, v_sizing(e_widget_sizing::ws_fill)
, h_align(e_widget_align::wa_left)
, v_align(e_widget_align::wa_top)
, b_fullscreen(_b_fullscreen)
, size(vector2d(1))
, min_size(vector2d(-1))
, bg_color(_bg_color)
, fg_color(_fg_color)
, b_hidden(false)
, b_ticks(false)
{
    if (b_fullscreen)
    {
        offset = vector2d(0, 0);
    }

    last_constraint = vector4d(-1);
    last_tick_time = time_now_ms();
    set_tick_rate(1000 / 30);   // 30 fps

    set_can_switch_to(true);
    set_draw_img_buffer(true);
    set_remove_img_artifacts_on_draw(false);
    set_take_draw_control(true);
}


void TermWidget::tick()
{
    if (b_ticks)
    {
        std::chrono::milliseconds time_now = time_now_ms();
        std::chrono::milliseconds delta = time_now - last_tick_time;
        if (delta >= tick_rate)
        {
            last_tick_time = time_now;
            std::chrono::milliseconds nulltime(0);
            if (delta > nulltime) tick_event(delta);
        }
    }
}


void TermWidget::set_tick_rate(float rate)
{
    tick_rate = std::chrono::milliseconds((int)rate);
}


std::string TermWidget::get_title() const
{
    if (title.empty())
    {
        return get_id();
    }

    return title;
}


void TermWidget::set_parent_widget(TermWidget* _parent_widget)
{
    // prevent setting child or sub-child as parent
    if (_parent_widget && !_parent_widget->is_child_of(this))
    {
        parent_widget = _parent_widget;
    }
    else if (!_parent_widget)
    {
        parent_widget = nullptr;
    }
}


bool TermWidget::is_child_of(TermWidget* wgt)
{
    if (this == wgt) return true;

    bool b_is = false;

    TermWidget* parent = get_parent_widget();
    while (parent)
    {
        if (parent == wgt)
        {
            b_is = true;
            break;
        }
        parent = parent->get_parent_widget();
    }

    return b_is;
}


bool TermWidget::handle_key_input(const tb_event& input_event, bool b_bubble_up)
{
    if (b_bubble_up && parent_widget)
    {
        return parent_widget->handle_key_input(input_event, true);
    }

    return false;
}


TermWidget* TermWidget::get_topmost_child_at(vector2d coord)
{
    // click within box
    vector2d abs = get_absolute_offset();
    vector2d dim = get_size();
    if (abs.x <= coord.x && abs.y <= coord.y &&
        abs.x + dim.x > coord.x && abs.y + dim.y > coord.y)
    {
        // click within child
        if (child_widget)
        {
            abs = child_widget->get_absolute_offset();
            dim = child_widget->get_size();
            if (abs.x <= coord.x && abs.y <= coord.y &&
                abs.x + dim.x > coord.x && abs.y + dim.y > coord.y)
            {
                return child_widget->get_topmost_child_at(coord);
            }
        }

        return this;
    }

    return nullptr;
}


int TermWidget::get_max_parent_width() const
{
    // get first widget above this widget in hierarchy
    // that does not have ws_auto, or term size
    TermWidget* wgt = parent_widget;
    while(wgt && wgt->get_h_sizing() == e_widget_sizing::ws_auto)
    {
        wgt = wgt->get_parent_widget();
    }

    if (wgt)
    {
        return wgt->get_width_constraint();
    }
    else
    {
        return term_w();
    }

    return 0;
}


int TermWidget::get_max_parent_height() const
{
    // get first widget above this widget in hierarchy
    // that does not have ws_auto, or term size
    TermWidget* wgt = parent_widget;
    while(wgt && wgt->get_h_sizing() == e_widget_sizing::ws_auto)
    {
        wgt = wgt->get_parent_widget();
    }

    if (wgt)
    {
        return wgt->get_height_constraint();
    }
    else
    {
        return term_h();
    }

    return 0;
}


int TermWidget::get_width_constraint() const
{
    int constraint = term_w();

    switch(h_sizing)
    {
        case ws_fullscreen  :
            constraint = term_w();
            break;

        case ws_fill        :
            {
                constraint = get_max_parent_width();
                vector4d padding = get_padding();
                constraint -= padding.a + padding.c;
            }
            break;

        // fill width is auto-calculated and set by parent
        case ws_fill_managed:
            constraint = size.x;
            break;

        case ws_auto        :
            {
                constraint = get_child_widget_size().x;
                vector4d padding = get_child_widget_padding();
                constraint += padding.a + padding.c;
            }
            break;

        case ws_fixed       :
            constraint = size.x;
            break;

        case ws_dynamic     :
            constraint = size.x;
            break;
    }

    // min size
    if (min_size.x != -1 &&
        constraint < min_size.x)
    {
        constraint = min_size.x;
    }

    return constraint;
}


int TermWidget::get_height_constraint() const
{
    int constraint = term_h();
    
    switch(v_sizing)
    {
        case ws_fullscreen  :
            constraint = term_h();
            break;

        case ws_fill        :
            {
                constraint = get_max_parent_height();
                vector4d padding = get_padding();
                constraint -= padding.b + padding.d;
            }
            break;

        // fill width is auto-calculated and set by parent
        case ws_fill_managed:
            constraint = size.y;
            break;

        case ws_auto        :
            {
                constraint = get_child_widget_size().y;
                vector4d padding = get_child_widget_padding();
                constraint += padding.b + padding.d;
            }
            break;

        case ws_fixed       :
            constraint = size.y;
            break;

        case ws_dynamic     :
            constraint = size.y;
            break;
    }

    // min size
    if (min_size.y != -1 &&
        constraint < min_size.y)
    {
        constraint = min_size.y;
    }

    return constraint;
}


void TermWidget::set_child_widget(std::shared_ptr<TermWidget> _child_widget, bool b_rebuild)
{
    // prevent parent widgets from being set as child widget
    if (_child_widget && !is_child_of(_child_widget.get()))
    {
        if (child_widget)
        {
            child_widget->set_parent_widget(nullptr);
        }

        _child_widget->set_parent_widget(this);
        _child_widget->set_inherited_padding(child_padding);
        _child_widget->update_size();
        child_widget = _child_widget;
    }
    else if (!_child_widget)
    {
        child_widget = nullptr;
    }

    if (b_rebuild)
    {
        rebuild();
    }
}


vector2d TermWidget::get_child_widget_size() const
{
    if (child_widget)
    {
        return child_widget->get_size();
    }

    // if no child widget, return size of self,
    // so that auto-sized widgets without children
    // use their own size as the width/height constraint
    return size;
}


vector4d TermWidget::get_child_widget_padding() const
{
    if (child_widget)
    {
        return child_widget->get_padding();
    }

    return vector4d();
}


void TermWidget::update_size(bool b_recursive)
{
    // cache the term size so we can conditionally call
    // update size only if the term size has changed
    term_size_cache = vector2d(term_w(), term_h());

    int w = get_width_constraint();
    int h = get_height_constraint();

    set_size(w, h);

    if (b_recursive)
    {
        update_child_size(b_recursive);
    }
}


void TermWidget::set_size(int w, int h)
{
    vector2d size_cache = size;

    if (b_fullscreen)
    {
        size.x = term_w();
        size.y = term_h();
    }
    else
    {
        if (w < 1) w = 1;
        size.x = w;

        if (h < 1) h = 1;
        size.y = h;
    }

    // min size
    if (min_size.x > -1 &&
        size.x < min_size.x)
    {
        size.x = min_size.x;
    }

    if (min_size.y > -1 &&
        size.y < min_size.y)
    {
        size.y = min_size.y;
    }

    if (size != size_cache)
    {
        size_change_event();
    }
}


void TermWidget::set_size(vector2d _size)
{
    set_size(_size.x, _size.y);
}


void TermWidget::refresh(bool b_draw_term)
{
    rebuild(true);
    if (b_draw_term)
    {
        WIDGET_MAN.draw_widgets();
    }
}


void TermWidget::handle_term_resize_event()
{
    update_size();
    rebuild(true);
}


vector2d TermWidget::get_alignment_offset() const
{
    vector2d off = vector2d();
    vector2d p_size;
    vector4d pad = get_padding();

    if (parent_widget)
    {
        p_size = parent_widget->get_size();
    }
    else
    {
        p_size = vector2d(term_w(), term_h());
    }

    switch (h_align)
    {
        case wa_left        :
            off.x = 0;
            break;

        case wa_center      :
            off.x = (float)(p_size.x - size.x) / 2.0;
            off.x -= pad.a;
            break;

        case wa_right       :
            off.x = p_size.x - size.x;
            break;
    }

    switch (v_align)
    {
        case wa_top         :
            off.y = 0;
            break;

        case wa_center      :
            off.y = (float)(p_size.y - size.y) / 2.0;
            off.y -= pad.b;
            break;

        case wa_bottom      :
            off.y = p_size.y - size.y;
            break;
    }

    return off;
}


vector2d TermWidget::get_absolute_offset() const
{
    if (parent_widget)
    {
        vector4d padding = get_padding();
        // left and top padding + offset of all parent widgets
        return
            offset +
            inherited_offset +
            vector2d(padding.a, padding.b) +
            parent_widget->get_absolute_offset() +
            get_alignment_offset();
    }
    else
    {
        vector4d padding = get_padding();
        return
            offset +
            inherited_offset +
            vector2d(padding.a, padding.b) +
            get_alignment_offset();
    }
}


vector4d TermWidget::get_padding() const
{
    return default_padding + inherited_padding;
}


void TermWidget::add_inherited_padding(vector4d pad)
{
    inherited_padding += pad;
}


void TermWidget::set_inherited_padding(vector4d pad)
{
    inherited_padding = pad;
}


void TermWidget::clear_inherited_padding()
{
    inherited_padding = vector4d();
}


void TermWidget::draw(vector4d constraint, bool b_draw_children) const
{
    if (b_hidden)
    {
        return;
    }

    // load constraint cache
    if (constraint.a == -1 && constraint.b == -1 &&
        constraint.c == -1 && constraint.d == -1)
    {
        constraint = last_constraint;
    }

    // x start
    if (constraint.a == -1) constraint.a = 0;
    // x end
    if (constraint.b == -1) constraint.b = term_w();
    // y start
    if (constraint.c == -1) constraint.c = 0;
    // y end
    if (constraint.d == -1) constraint.d = term_h();

    // do not cache term dimensions
    vector4d cons = constraint;
    if (cons.b == term_w())
    {
        cons.b = -1;
    }
    if (cons.d == term_h())
    {
        cons.d = -1;
    }
    last_constraint = cons;

    vector2d absolute_offset = get_absolute_offset();
    term_cell cell;

    // only draw cells that are visible
    for (int y = constraint.c; y < constraint.d; ++y)
    {
        int cell_row = -absolute_offset.y + y;
        if (cell_row < cells.size())
        {
            const std::vector<tb_cell>& row = cells[cell_row];
            for (int x = constraint.a; x < constraint.b; ++x)
            {
                int cell_col = -absolute_offset.x + x;
                if (cell_col < row.size())
                {
                    cell.tb_data = row[cell_col];
                    cell.coord.x = x;
                    cell.coord.y = y;
                    WIDGET_MAN.add_cell_to_frame(cell);

                    // optionally blast out image artifacts
                    // where widget will be drawn
                    if (b_remove_img_artifacts_on_draw)
                    {
                        WIDGET_MAN.remove_image_artifact(cell.coord);
                    }
                }
            }
        }
    }

    if (b_draw_children) draw_children(constraint);
}


void TermWidget::draw_children(vector4d constraint) const
{
    if (child_widget)
    {
        child_widget->draw(constraint, true);
    }
}


vector2d TermWidget::size_on_screen() const
{
    vector2d draw_size;
    int largest_x = 0;
    vector2d absolute_offset = get_absolute_offset();

    // if this widget is a container widget,
    // get size of child (recursively)
    if (cells.size() == 0)
    {
        if (child_widget)
        {
            return child_widget->size_on_screen();
        }
    }
    else
    {
        // only draw cells that are visible
        for (int y = 0; y < term_h(); ++y)
        {
            int cell_row = -absolute_offset.y + y;
            if (cell_row < cells.size())
            {
                const std::vector<tb_cell>& row = cells[cell_row];
                if (row.size() > largest_x)
                {
                    largest_x = row.size();
                }

                draw_size.y++;
            }
        }

        draw_size.x = largest_x;
    }

    return draw_size;
}


void TermWidget::on_focus_received()
{
    // only update size and rebuild if term size has changed
    if (term_size_cache != vector2d(term_w(), term_h()))
    {
        update_size();
        rebuild(true);
    }

    show();

    WIDGET_MAN.termbox_clear(COLO.img_artifact_remove);
    WIDGET_MAN.termbox_draw();
    WIDGET_MAN.draw_widgets();
}


void TermWidget::on_focus_lost()
{
    hide();
}


int TermWidget::term_h()
{
    return WIDGET_MAN.get_term_size().y;
}


int TermWidget::term_w()
{
    return WIDGET_MAN.get_term_size().x;
}


void TermWidget::delete_self()
{
    WIDGET_MAN.remove_widget(this);
}


void TermWidget::receive_left_click(vector2d coord, TermWidget* clicked, TermWidget* source)
{
    // bubble up
    if (parent_widget)
    {
        if (!clicked) clicked = this;
        parent_widget->receive_left_click(coord, clicked, this);
    }
}


void TermWidget::receive_right_click(TermWidget* clicked, TermWidget* source)
{
    // bubble up
    if (parent_widget)
    {
        if (!clicked) clicked = this;
        parent_widget->receive_right_click(source, this);
    }
}


void TermWidget::receive_middle_click(TermWidget* clicked, TermWidget* source)
{
    // bubble up
    if (parent_widget)
    {
        if (!clicked) clicked = this;
        parent_widget->receive_middle_click(clicked, this);
    }
}

