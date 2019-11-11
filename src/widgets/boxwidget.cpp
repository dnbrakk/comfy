/**
 * Comfy
 *
 *  Copyright 2019 by Wolfish <wolfish@airmail.cc>
 *  https://wolfish.neocities.org/soft/comfy/
 *
 *  Licensed under the GPL v2.0 only.
 */
#include "boxwidget.h"


BoxWidget::BoxWidget(vector2d _offset, vector4d _padding, vector2d _size, uint32_t _bg_color, uint32_t _fg_color, bool _b_fullscreen)
: TermWidget(_offset, _padding, vector4d(2, 1, 2, 1), _bg_color, _fg_color, _b_fullscreen)
, b_draw_border(true)
{
    set_size(_size);
    set_use_alt_color(false);
    rebuild();
}


void BoxWidget::rebuild(bool b_rebuild_children)
{
    if (child_widget)
    {
        if (b_rebuild_children)
        {
            child_widget->rebuild();
        }

        update_size(false /* recursive */);
    }

    cells.clear();

    tb_cell cell;
    std::vector<tb_cell> row;

    if (b_draw_border)
    {
        // top left corner
        cell.ch = L'\u250C';
        if (b_use_alt_color &&
            alt_color_start.y == 0 && alt_color_start.x == 0)
        {
            cell.bg = alt_bg_color;
            cell.fg = alt_fg_color;
        }
        else
        {
            cell.bg = bg_color;
            cell.fg = fg_color;
        }
        row.push_back(cell);

        // top edge
        for (int i = 0; i < size.x - 2; ++i)
        {
            cell.ch = L'\u2500';
            if (b_use_alt_color &&
                alt_color_start.y == 0 && i + 1 >= alt_color_start.x)
            {
                cell.bg = alt_bg_color;
                cell.fg = alt_fg_color;
            }
            else
            {
                cell.bg = bg_color;
                cell.fg = fg_color;
            }
            row.push_back(cell);
        }
        
        // top right corner
        cell.ch = L'\u2510';
        if (b_use_alt_color && 
            alt_color_start.y == 0 && alt_color_start.x <= size.x - 1)
        {
            cell.bg = alt_bg_color;
            cell.fg = alt_fg_color;
        }
        else
        {
            cell.bg = bg_color;
            cell.fg = fg_color;
        }
        row.push_back(cell);

        cells.push_back(row);
        row.clear();
    }
    
    int x = size.x;
    int y = size.y;
    if (b_draw_border)
    {
        x -= 2;
        y -= 2;
    }

    // sides
    for (int j = 0; j < y; ++j)
    {
        if (b_draw_border)
        {
            // left side
            cell.ch = L'\u2502';
            if (b_use_alt_color && 
                j >= alt_color_start.y && alt_color_start.x == 0)
            {
                cell.bg = alt_bg_color;
                cell.fg = alt_fg_color;
            }
            else
            {
                cell.bg = bg_color;
                cell.fg = fg_color;
            }
            row.push_back(cell);
        }

        // whitespace
        for (int k = 0; k < x; ++k)
        {
            cell.ch = L' ';
            if (b_use_alt_color && 
                j >= alt_color_start.y && k >= alt_color_start.x)
            {
                cell.bg = alt_bg_color;
                cell.fg = alt_fg_color;
            }
            else
            {
                cell.bg = bg_color;
                cell.fg = fg_color;
            }
            row.push_back(cell);
        }

        if (b_draw_border)
        {
            // right side
            cell.ch = L'\u2502';
            if (b_use_alt_color && 
                j >= alt_color_start.y && alt_color_start.x <= size.x - 1)
            {
                cell.bg = alt_bg_color;
                cell.fg = alt_fg_color;
            }
            else
            {
                cell.bg = bg_color;
                cell.fg = fg_color;
            }
            row.push_back(cell);
        }

        cells.push_back(row);
        row.clear();
    }

    if (b_draw_border)
    {
        // bottom left corner
        cell.ch = L'\u2514';
        if (b_use_alt_color &&
            alt_color_start.y <= size.y - 1 && alt_color_start.x == 0)
        {
            cell.bg = alt_bg_color;
            cell.fg = alt_fg_color;
        }
        else
        {
            cell.bg = bg_color;
            cell.fg = fg_color;
        }
        row.push_back(cell);

        // bottom edge
        for (int i = 0; i < size.x - 2; ++i)
        {
            cell.ch = L'\u2500';
            if (b_use_alt_color &&
                alt_color_start.y <= size.y - 1 && i + 1 >= alt_color_start.x)
            {
                cell.bg = alt_bg_color;
                cell.fg = alt_fg_color;
            }
            else
            {
                cell.bg = bg_color;
                cell.fg = fg_color;
            }
            row.push_back(cell);
        }
        
        // bottom right corner
        cell.ch = L'\u2518';
        if (b_use_alt_color && 
            alt_color_start.y <= size.y && alt_color_start.x <= size.x - 1)
        {
            cell.bg = alt_bg_color;
            cell.fg = alt_fg_color;
        }
        else
        {
            cell.bg = bg_color;
            cell.fg = fg_color;
        }
        row.push_back(cell);

        cells.push_back(row);
    }
}


void BoxWidget::child_widget_size_change_event()
{
    rebuild(false);
}


vector4d BoxWidget::get_child_widget_padding() const
{
    if (child_widget)
    {
        return child_widget->get_padding();
    }

    return vector4d();
}


vector2d BoxWidget::get_child_widget_size() const
{
    if (child_widget)
    {
        return child_widget->get_size();
    }

    return vector2d();
}


void BoxWidget::update_child_size(bool b_recursive)
{
    if (child_widget)
    {
        child_widget->update_size(b_recursive);
    }
}

