/**
 * Comfy
 *
 *  Copyright 2019 by Wolfish <wolfish@airmail.cc>
 *  https://wolfish.neocities.org/soft/comfy/
 *
 *  Licensed under the GPL v2.0 only.
 */
#include "colorblockwidget.h"


ColorBlockWidget::ColorBlockWidget(uint16_t _color, bool _b_fullscreen, vector2d _offset, vector4d _padding, vector2d _size)
: TermWidget(_offset, _padding, vector4d(), _color, _color, _b_fullscreen)
{
    if (b_fullscreen)
    {
        set_h_sizing(e_widget_sizing::ws_fullscreen);
        set_v_sizing(e_widget_sizing::ws_fullscreen);
    }
    else
    {
        set_h_sizing(e_widget_sizing::ws_fixed);
        set_v_sizing(e_widget_sizing::ws_fixed);
    }

    set_size(_size);
    rebuild();
}


void ColorBlockWidget::rebuild(bool b_rebuild_children)
{
    cells.clear();

    update_size();

    for (int y = 0; y < size.y; ++y)
    {
        std::vector<tb_cell> row;

        for (int x = 0; x < size.x; ++x)
        {
            tb_cell cell;
            uint32_t unicode_char;
            tb_utf8_char_to_unicode(&unicode_char, " ");
            cell.ch = unicode_char;
            cell.bg = bg_color;
            cell.fg = bg_color;
            row.push_back(cell);
        }

        cells.push_back(row);
    }
}

