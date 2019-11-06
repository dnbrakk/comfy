/**
 * Comfy
 *
 *  Copyright 2019 by Wolfish <wolfish@airmail.cc>
 *  https://wolfish.neocities.org/soft/comfy/
 *
 *  Licensed under the GPL v2.0 only.
 */
#include "multichildwidget.h"
#include "../widgetman.h"


MultiChildWidget::MultiChildWidget(vector2d _offset, vector4d _padding, vector4d _child_padding, uint32_t _bg_color, uint32_t _fg_color, bool _b_fullscreen)
: TermWidget(_offset, _padding, _child_padding, _bg_color, _fg_color, _b_fullscreen)
{}


void MultiChildWidget::add_child_widget(std::shared_ptr<TermWidget> child_widget, bool b_rebuild)
{
    // prevent parent widgets from being set as child widget
    if (child_widget && !is_child_of(child_widget.get()))
    {
        set_managed_sizing(child_widget);

        children.push_back(child_widget);
        child_widget->set_parent_widget(this);
        child_widget->update_size();

        if (b_rebuild)
        {
            rebuild();
        }
    }
}


TermWidget* MultiChildWidget::get_topmost_child_at(vector2d coord)
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


void MultiChildWidget::child_widget_size_change_event()
{
    rebuild(false);
}


void MultiChildWidget::draw_children(vector4d constraint) const
{
    for (const auto& child : children)
    {
        if (child)
        {
            child->draw(constraint, true);
        }
    }
}


void MultiChildWidget::update_child_size(bool b_recursive)
{
    for (auto& child : children)
    {
        if (child)
        {
            child->update_size(b_recursive);
        }
    }
}


int MultiChildWidget::num_children() const
{
    return children.size();
}

