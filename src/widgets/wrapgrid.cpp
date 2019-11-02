/**
 * Comfy
 *
 *  Copyright 2019 by Wolfish <wolfish@airmail.cc>
 *  https://wolfish.neocities.org/soft/comfy/
 *
 *  Licensed under the GPL v2.0 only.
 */
#include "wrapgrid.h"
#include "../widgetman.h"


WrapGrid::WrapGrid(
    vector2d _slot_size,
    vector2d _offset,
    vector4d _padding,
    vector4d _child_padding
)
: TermWidget(_offset, _padding, _child_padding)
, slot_size(_slot_size)
{
    set_h_sizing(ws_fill);
}


void WrapGrid::rebuild(bool b_rebuild_children)
{
    cells.clear();

    if (b_rebuild_children)
    {
        for (auto& child : children)
        {
            if (child) child->rebuild();
        }
    }

    int w = get_width_constraint();

    int width = 0;
    int largest_width = 0;
    int height = 0;
    for (auto& child : children)
    {
        if (child)
        {
            if (width + slot_size.x > w)
            {
                width = 0;
                height += slot_size.y;
            }

            child->set_inherited_offset(vector2d(width, height));
            width += slot_size.x;
            if (width > largest_width)
            {
                largest_width = width;
            }
        }
    }

    set_size(largest_width, height + slot_size.y);
}


void WrapGrid::add_child_widget(std::shared_ptr<TermWidget> child_widget, bool b_rebuild)
{
    if (child_widget)
    {
        // set size of children manually
        child_widget->set_h_sizing(ws_fixed);
        child_widget->set_v_sizing(ws_fixed);
        child_widget->set_size(slot_size);

        children.push_back(child_widget);
        child_widget->set_parent_widget(this);
        child_widget->update_size();

        if (b_rebuild)
        {
            rebuild();
        }
    }
}


TermWidget* WrapGrid::get_topmost_child_at(vector2d coord)
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


void WrapGrid::child_widget_size_change_event()
{
    rebuild(false);
}


void WrapGrid::draw_children(vector4d constraint) const
{
    for (const auto& child : children)
    {
        if (child)
        {
            child->draw(constraint);
        }
    }
}


vector2d WrapGrid::get_child_widget_size() const
{
    vector2d out;

    for (auto& child : children)
    {
        if (child)
        {
            vector2d c_size = child->get_size();
            out.x += c_size.x;

            if (c_size.y > out.y)
            {
                out.y = child->get_size().y;
            }
        }
    }

    return out;
}


void WrapGrid::update_child_size(bool b_recursive)
{
    for (auto& child : children)
    {
        if (child)
        {
            child->update_size(b_recursive);
        }
    }
}


vector2d WrapGrid::size_on_screen() const
{
    vector2d draw_size;
    int largest_y = 0;

    for (const auto& child : children)
    {
        if (child)
        {
            vector2d cs = child->size_on_screen();
            if (cs.y > largest_y)
            {
                largest_y = cs.y;
            }

            draw_size.x += cs.x;
        }
    }

    draw_size.y = largest_y;
    return draw_size;
}

