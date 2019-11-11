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
: MultiChildWidget(_offset, _padding)
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
            if (child) child->rebuild(b_rebuild_children);
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

    if (get_h_sizing() == ws_auto)
    {
        set_size(width, height);
    }
    else
    {
        set_size(get_width_constraint(), height);
    }
}


void VerticalBoxWidget::set_managed_sizing(std::shared_ptr<TermWidget> wgt)
{
    if (!wgt) return;

    if (wgt->get_v_sizing() == ws_fullscreen ||
        wgt->get_v_sizing() == ws_fill)
    {
        wgt->set_v_sizing(ws_fill_managed);
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

