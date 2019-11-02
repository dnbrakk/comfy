/**
 * Comfy
 *
 *  Copyright 2019 by Wolfish <wolfish@airmail.cc>
 *  https://wolfish.neocities.org/soft/comfy/
 *
 *  Licensed under the GPL v2.0 only.
 */
#include "verticalboxwidget.h"
#include "../widgetman.h"


VerticalBoxWidget::VerticalBoxWidget(bool _b_stretch_offscreen, vector2d _offset, vector4d _padding)
: TermWidget(_offset, _padding)
, b_stretch_offscreen(_b_stretch_offscreen)
{
}


void VerticalBoxWidget::rebuild(bool b_rebuild_children)
{
    cells.clear();

    if (b_rebuild_children)
    {
        for (auto& child : children)
        {
            if (child) child->rebuild();
        }

        int h = get_height_constraint();

        int fixed_h = 0;
        std::vector<TermWidget*> fill;
        for (auto& child : children)
        {
            if (child)
            {
                if (child->get_v_sizing() == ws_fill_managed)
                {
                    fill.push_back(child.get());
                }
                else
                {
                    vector4d pad = child->get_padding();
                    fixed_h += child->get_height_constraint() + pad.b + pad.d;
                }
            }
        }

        if (fill.size() > 0 && h > fixed_h)
        {
            // divide space up between fill widgets
            int rem = h - fixed_h;
            int div = (float)rem / (float)fill.size();
            int _h = tb_height();
            std::vector<TermWidget*> changed;
            for (auto& child : fill)
            {
                if (child)
                {
                    vector2d c_size = child->get_size();
                    vector4d c_pad = child->get_padding();
                    int new_h = div - c_pad.b - c_pad.d;
                    if (new_h != c_size.y)
                    {
                        child->set_size(c_size.x, new_h);
                        changed.push_back(child);
                    }
                }
            }

            // rebuild children whose sizes have changed
            for (auto& child : changed)
            {
                if (child) child->rebuild();
            }
        }
    }

    int width = 0;
    int height = 0;
    for (auto& child : children)
    {
        if (child)
        {
            child->set_inherited_offset(vector2d(0, height));

            vector4d pad = child->get_padding();
            if (child->get_size().x + pad.a + pad.c > width)
            {
                width = child->get_size().x + pad.a + pad.c;
            }

            height += child->get_size().y + pad.b + pad.d;
        }
    }

    set_size(width, height);
}


void VerticalBoxWidget::add_child_widget(std::shared_ptr<TermWidget> child_widget, bool b_rebuild)
{
    if (child_widget)
    {
        if (child_widget->get_v_sizing() == ws_fullscreen ||
            child_widget->get_v_sizing() == ws_fill)
        {
            child_widget->set_v_sizing(ws_fill_managed);
        }

        children.push_back(child_widget);
        child_widget->set_parent_widget(this);
        child_widget->update_size();

        if (b_rebuild)
        {
            rebuild();
        }
    }
}


TermWidget* VerticalBoxWidget::get_topmost_child_at(vector2d coord)
{
    // click within box
    vector2d abs = get_absolute_offset();
    vector2d dim = get_size();
    if (abs.x <= coord.x && abs.y <= coord.y &&
        abs.x + dim.x > coord.x && abs.y + dim.y > coord.y)
    {
        // click within child
        for (auto& child : children)
        {
            if (child)
            {
                abs = child->get_absolute_offset();
                dim = child->get_size();
                if (abs.x <= coord.x && abs.y <= coord.y &&
                    abs.x + dim.x > coord.x && abs.y + dim.y > coord.y)
                {
                    return child->get_topmost_child_at(coord);
                }
            }
        }

        return this;
    }
}


void VerticalBoxWidget::child_widget_size_change_event()
{
    rebuild(false);
}


void VerticalBoxWidget::draw_children(vector4d constraint) const
{
    for (const auto& child : children)
    {
        if (child)
        {
            child->draw(constraint);
        }
    }
}


vector2d VerticalBoxWidget::get_child_widget_size() const
{
    vector2d out;

    for (auto& child : children)
    {
        if (child)
        {
            vector2d c_size = child->get_size();
            out.y += c_size.y;

            if (c_size.x > out.x)
            {
                out.x = child->get_size().x;
            }
        }
    }

    return out;
}


void VerticalBoxWidget::update_child_size(bool b_recursive)
{
    for (auto& child : children)
    {
        if (child)
        {
            child->update_size(b_recursive);
        }
    }
}


vector2d VerticalBoxWidget::size_on_screen() const
{
    vector2d draw_size;
    int largest_x = 0;

    for (const auto& child : children)
    {
        if (child)
        {
            vector2d cs = child->size_on_screen();
            if (cs.x > largest_x)
            {
                largest_x = cs.x;
            }

            draw_size.y += cs.y;
        }
    }

    draw_size.x = largest_x;
    return draw_size;
}


int VerticalBoxWidget::num_children() const
{
    return children.size();
}

