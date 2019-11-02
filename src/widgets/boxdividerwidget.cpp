/**
 * Comfy
 *
 *  Copyright 2019 by Wolfish <wolfish@airmail.cc>
 *  https://wolfish.neocities.org/soft/comfy/
 *
 *  Licensed under the GPL v2.0 only.
 */
#include "boxdividerwidget.h"


BoxDividerWidget::BoxDividerWidget(bool _b_horizontal, uint32_t _bg_color = 0, uint32_t _fg_color = 15)
: TermWidget(vector2d(), vector4d(), vector4d(), _bg_color, _fg_color, false)
, b_horizontal(_b_horizontal)
{
}


void BoxDividerWidget::rebuild(bool b_rebuild_children)
{
    cells.clear();

    if (!parent_widget)
    {
        return;
    }

    int p_size;
    tb_cell cell;
    uint32_t unicode_char;
    std::vector<tb_cell> row;

    if (b_horizontal)
    {
        offset = vector2d(-2, 0);
        inherited_padding = vector4d();
        p_size = parent_widget->get_width_constraint() + 2;
        set_size(p_size, 1);

        // left edge
        tb_utf8_char_to_unicode(&unicode_char, "\u251C");
        cell.ch = unicode_char;
        cell.bg = bg_color;
        cell.fg = fg_color;
        row.push_back(cell);

        // line
        for (int i = 0; i < p_size; ++i)
        {
            tb_utf8_char_to_unicode(&unicode_char, "\u2500");
            cell.ch = unicode_char;
            cell.bg = bg_color;
            cell.fg = fg_color;
            row.push_back(cell);
        }
        
        // right edge
        tb_utf8_char_to_unicode(&unicode_char, "\u2524");
        cell.ch = unicode_char;
        cell.bg = bg_color;
        cell.fg = fg_color;
        row.push_back(cell);

        cells.push_back(row);
    }
    else
    {
        p_size = parent_widget->get_height_constraint();
        set_size(1, p_size);

        // top edge
        tb_utf8_char_to_unicode(&unicode_char, "\u252C");
        cell.ch = unicode_char;
        cell.bg = bg_color;
        cell.fg = fg_color;

        row.push_back(cell);
        cells.push_back(row);
        row.clear();

        // line
        for (int k = 0; k < p_size + 1; ++k)
        {
            tb_utf8_char_to_unicode(&unicode_char, "\u2502");
            cell.ch = unicode_char;
            cell.bg = bg_color;
            cell.fg = bg_color;

            row.push_back(cell);
            cells.push_back(row);
            row.clear();
        }

        // bottom edge
        tb_utf8_char_to_unicode(&unicode_char, "\u2534");
        cell.ch = unicode_char;
        cell.bg = bg_color;
        cell.fg = fg_color;

        row.push_back(cell);
        cells.push_back(row);
    }
}

