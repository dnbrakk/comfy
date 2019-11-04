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
    uint32_t unicode_char;
    std::vector<tb_cell> row;

    if (b_draw_border)
    {
        // top left corner
        tb_utf8_char_to_unicode(&unicode_char, "\u250C");
        cell.ch = unicode_char;
        cell.bg = bg_color;
        cell.fg = fg_color;
        row.push_back(cell);

        // top edge
        for (int i = 0; i < size.x - 2; ++i)
        {
            tb_utf8_char_to_unicode(&unicode_char, "\u2500");
            cell.ch = unicode_char;
            cell.bg = bg_color;
            cell.fg = fg_color;
            row.push_back(cell);
        }
        
        // top right corner
        tb_utf8_char_to_unicode(&unicode_char, "\u2510");
        cell.ch = unicode_char;
        cell.bg = bg_color;
        cell.fg = fg_color;
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
            tb_utf8_char_to_unicode(&unicode_char, "\u2502");
            cell.ch = unicode_char;
            cell.bg = bg_color;
            cell.fg = fg_color;
            row.push_back(cell);
        }

        // whitespace
        for (int k = 0; k < x; ++k)
        {
            tb_utf8_char_to_unicode(&unicode_char, " ");
            cell.ch = unicode_char;
            cell.bg = bg_color;
            cell.fg = bg_color;
            row.push_back(cell);
        }

        if (b_draw_border)
        {
            // right side
            tb_utf8_char_to_unicode(&unicode_char, "\u2502");
            cell.ch = unicode_char;
            cell.bg = bg_color;
            cell.fg = fg_color;
            row.push_back(cell);
        }

        cells.push_back(row);
        row.clear();
    }

    if (b_draw_border)
    {
        // bottom left corner
        tb_utf8_char_to_unicode(&unicode_char, "\u2514");
        cell.ch = unicode_char;
        cell.bg = bg_color;
        cell.fg = fg_color;
        row.push_back(cell);

        // bottom edge
        for (int i = 0; i < size.x - 2; ++i)
        {
            tb_utf8_char_to_unicode(&unicode_char, "\u2500");
            cell.ch = unicode_char;
            cell.bg = bg_color;
            cell.fg = fg_color;
            row.push_back(cell);
        }
        
        // bottom right corner
        tb_utf8_char_to_unicode(&unicode_char, "\u2518");
        cell.ch = unicode_char;
        cell.bg = bg_color;
        cell.fg = fg_color;
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

