/**
 * Comfy
 *
 *  Copyright 2019 by Wolfish <wolfish@airmail.cc>
 *  https://wolfish.neocities.org/soft/comfy/
 *
 *  Licensed under the GPL v2.0 only.
 */
#include "horizontalboxwidget.h"
#include "../widgetman.h"


HorizontalBoxWidget::HorizontalBoxWidget(vector2d _offset, vector4d _padding, vector4d _child_padding)
: MultiChildWidget(_offset, _padding, _child_padding)
{
    set_h_sizing(ws_fill);
}


void HorizontalBoxWidget::rebuild(bool b_rebuild_children)
{
    cells.clear();

    for (auto& child : children)
    {
        if (child) child->rebuild();
    }

    int w = get_width_constraint();

    int fixed_w = 0;
    std::vector<TermWidget*> fill;
    for (auto& child : children)
    {
        if (child)
        {
            if (child->get_h_sizing() == ws_fill_managed)
            {
                fill.push_back(child.get());
            }
            else
            {
                fixed_w += child->get_width_constraint() +
                    child_padding.a + child_padding.c;
            }
        }
    }

    if (fill.size() > 0 && w > fixed_w)
    {
        // divide space up between fill widgets
        bool b_changed = false;
        int rem = w - fixed_w;
        int div = (float)rem / (float)fill.size();
        for (auto& child : fill)
        {
            if (child)
            {
                vector2d c_size = child->get_size();
                vector4d c_pad = child->get_padding();
                int new_w = div - c_pad.a - c_pad.c;
                if (new_w != c_size.x)
                {
                    child->set_size(new_w, c_size.y);
                    b_changed = true;
                }
            }
        }

        // rebuild children again as their sizes have changed
        if (b_changed)
        {
            for (auto& child : children)
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
            child->set_inherited_offset(vector2d(width, 0));

            vector4d pad = child->get_padding();
            if (child->get_size().y + pad.b + pad.d > height)
            {
                height = child->get_size().y + pad.b + pad.d;
            }

            width += child->get_size().x + pad.a + pad.c +
                child_padding.a + child_padding.c;
        }
    }

    set_size(width, height);
}


void HorizontalBoxWidget::set_managed_sizing(std::shared_ptr<TermWidget> wgt)
{
    if (!wgt) return;

    if (wgt->get_h_sizing() == ws_fullscreen ||
        wgt->get_h_sizing() == ws_fill)
    {
        wgt->set_h_sizing(ws_fill_managed);
    }
}


vector2d HorizontalBoxWidget::get_child_widget_size() const
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


vector2d HorizontalBoxWidget::size_on_screen() const
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

