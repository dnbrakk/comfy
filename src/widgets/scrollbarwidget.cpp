/**
 * Comfy
 *
 *  Copyright 2019 by Wolfish <wolfish@airmail.cc>
 *  https://wolfish.neocities.org/soft/comfy/
 *
 *  Licensed under the GPL v2.0 only.
 */
#include "scrollbarwidget.h"
#include "../widgetman.h"
#include "scrollpanelwidget.h"


ScrollBarWidget::ScrollBarWidget(ScrollPanelWidget* _parent_panel, uint32_t _bg_color, uint32_t _fg_color)
: TermWidget(vector2d(), vector4d(), vector4d(), _bg_color, _fg_color, false)
, parent_panel(_parent_panel)
{
    set_h_sizing(e_widget_sizing::ws_fixed);
    set_v_sizing(e_widget_sizing::ws_fixed);
    set_size(vector2d(1));
}


void ScrollBarWidget::rebuild(bool b_rebuild_children)
{
    if (!parent_panel)
    {
        return;
    }

    // offset on screen, minus panel's offset as it is the
    // offset used to scroll the panel off screen
    vector2d p_offset = parent_panel->get_absolute_offset() - parent_panel->get_offset();
    vector2d p_size = parent_panel->get_size();
    vector4d p_pad = parent_panel->get_padding();
    doc_size = p_size.y + p_pad.b + p_pad.d;
    // panel offset is scroll position
    doc_position = -parent_panel->get_offset().y;
    // right-hand side
    set_offset(vector2d(p_size.x - 1, p_offset.y));
    // height is height of parent scroll panel
    size.y = parent_panel->get_visible_height();

    if (doc_size < 1)
    {
        doc_size = 1;
    }

    if (doc_position < 0)
    {
        doc_position = 0;
    }

    cells.clear();

    tb_cell cell;
    uint32_t unicode_char;
    tb_utf8_char_to_unicode(&unicode_char, " ");
    std::vector<tb_cell> row;

    float doc_size_float = (float)doc_size;
    float doc_position_float = (float)doc_position;
    float size_y = (float)size.y;

    float slider_ratio = size_y / doc_size_float;
    float slider_size_float = size_y * slider_ratio;

    int slider_size = (int)slider_size_float;
    if (slider_size < 1)
    {
        slider_size = 1;
    }
    else if (slider_size > size.y)
    {
        slider_size = size.y;
    }

    doc_size_float = (float)(doc_size - size.y);
    float slider_position_float = (size_y - slider_size - 1) * (doc_position_float / doc_size_float);
    int slider_position = (int)slider_position_float;
    if (slider_position < 0)
    {
        slider_position = 0;
    }
    // make sure slider is not at top if page is scrolled any amount
    else if (slider_position_float > 0.0f)
    {
        slider_position += 1;
    }

    // prevent slider from moving off-screen
    if (doc_position >= doc_size - size.y)
    {
        slider_position = size.y - slider_size - 1;
    }

    // make sure slider is not at bottom if page is not max scrolled
    if (doc_position < doc_size - size.y &&
        slider_position == size.y - slider_size - 1)
    {
        slider_position--;
    }

    for (int i = 0; i < size.y; ++i)
    {
        row.clear();
        if (i >= slider_position && i <= slider_position + slider_size)
        {
            tb_cell cell;
            cell.ch = unicode_char;
            cell.bg = fg_color;
            cell.fg = fg_color;
            row.push_back(cell);
            cells.push_back(row);
        }
        else
        {
            tb_cell cell;
            cell.ch = unicode_char;
            cell.bg = bg_color;
            cell.fg = bg_color;
            row.push_back(cell);
            cells.push_back(row);
        }
    }
}

