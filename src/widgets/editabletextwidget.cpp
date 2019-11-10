/**
 * Comfy
 *
 *  Copyright 2019 by Wolfish <wolfish@airmail.cc>
 *  https://wolfish.neocities.org/soft/comfy/
 *
 *  Licensed under the GPL v2.0 only.
 */
#include "editabletextwidget.h"
#include "../widgetman.h"


EditableTextWidget::EditableTextWidget(vector2d _offset, vector4d _padding, uint32_t _bg_color, uint32_t _fg_color)
: TermWidget(_offset, _padding, vector4d(), _bg_color, _fg_color)
{
    set_h_align(e_widget_align::wa_left);
    set_v_align(e_widget_align::wa_top);
    set_h_sizing(e_widget_sizing::ws_fill);
    set_v_sizing(e_widget_sizing::ws_dynamic);
    set_draw_cursor(true);
    widest_row = 0;
    cursor_pos = 0;
    up_down_w_cache = 0;
}



// returns true if max height has been reached
void EditableTextWidget::push_row(std::vector<tb_cell>& row)
{
    cells.push_back(row);
    if (row.size() > widest_row)
    {
        widest_row = row.size();
    }
    row.clear(); 
}


void EditableTextWidget::make_cells(std::vector<tb_cell>& row, const std::wstring& text)
{
    for (auto& ch : text)
    {
        tb_cell cell;
        cell.ch = ch;
        cell.bg = bg_color;
        cell.fg = fg_color;

        row.push_back(cell);
    }
}


void EditableTextWidget::rebuild(bool b_rebuild_children)
{
    cells.clear();
    index_coord_map.clear();
    coord_index_map.clear();
    widest_row = 0;

    int max_width = get_width_constraint();
    std::vector<tb_cell> row;

    int index = 0;
    int width = 0;
    for (int i = 0; i < buf.length(); ++i)
    {
        const wchar_t& ch = buf[i];

        if (width >= max_width)
        {
            push_row(row);
            width = 0;
        }

        if (ch == L'\n')
        {
            push_row(row);
            width = -1;
        }
        else
        {
            tb_cell cell;
            cell.ch = ch;
            cell.bg = bg_color;
            cell.fg = fg_color;

            row.push_back(cell);
        }

        index_coord_map[index] = vector2d(width, cells.size());
        coord_index_map[vector2d(width, cells.size())] = index; 

        // look ahead to see if word wrap is needed
        if (ch == L' ')
        {
            for (int j = i + 1; j < buf.length(); ++j)
            {
                const wchar_t& ch2 = buf[j];
                if (ch2 == L' ' || ch2 == L'\n')
                    break;

                // next word exceeds max width, so wrap
                if (j - index + width + 2 >= max_width)
                {
                    coord_index_map.erase(vector2d(width, cells.size()));
                    push_row(row);
                    width = -1;
                    index_coord_map[index] = vector2d(width, cells.size());
                    coord_index_map[vector2d(width, cells.size())] = index; 
                    break;
                }
            }
        }

        index++;
        width++;
    }

    if (row.size() > 0)
    {
        push_row(row);
    }

    set_size(max_width, cells.size());
}



void EditableTextWidget::insert_char(wchar_t ch, bool b_rebuild)
{
    if (cursor_pos < 0)
        cursor_pos = 0;

    if (buf.size() > 0)
    {
        if (cursor_pos < buf.length())
        {
            buf.insert(cursor_pos, 1, ch);
        }
        else
        {
            buf.append(1, ch);
        }
    }
    else
    {
        buf = std::wstring(1, ch);
    }

    cursor_pos++;
    up_down_w_cache = 0;

    if (b_rebuild)
        rebuild();
}


void EditableTextWidget::remove_char(bool b_rebuild)
{
    if (buf.empty() || cursor_pos - 1 < 0)
        return;

    if (buf.size() > 1)
    {
        if (cursor_pos < buf.length())
        {
            buf = buf.substr(0, cursor_pos - 1) + buf.substr(cursor_pos);
        }
        else
        {
            buf = buf.substr(0, cursor_pos - 1);
        }
    }
    else
    {
        buf = std::wstring();
    }

    cursor_pos--;
    up_down_w_cache = 0;

    if (b_rebuild)
        rebuild();
}


void EditableTextWidget::move_cursor_left()
{
    if (cursor_pos < 0)
    {
        cursor_pos = 0;
        return;
    }

    if (cursor_pos == 0)
        return;

    cursor_pos--;
    up_down_w_cache = 0;
}


void EditableTextWidget::move_cursor_right()
{
    if (cursor_pos > buf.length())
    {
        cursor_pos = buf.length();
        return;
    }

    if (cursor_pos == buf.length())
        return;

    cursor_pos++;
    up_down_w_cache = 0;
}


void EditableTextWidget::move_cursor_up()
{
    if (cursor_pos < 1)
    {
        up_down_w_cache = 0;
        return;
    }

    vector2d coord = get_coord_at_index(cursor_pos - 1);
    coord.y--;

    if (coord.y == -1)
    {
        up_down_w_cache = 0;
    }
    else if (coord.x > up_down_w_cache)
    {
        up_down_w_cache = coord.x;
    }
    else if (up_down_w_cache > 0)
    {
        coord.x = up_down_w_cache;
    }

    cursor_pos = get_index_at_coord(coord) + 1;
}


void EditableTextWidget::move_cursor_down()
{
    if (cursor_pos == buf.length())
    {
        up_down_w_cache = 0;
        return;
    }

    vector2d coord = get_coord_at_index(cursor_pos - 1);
    coord.y++;

    if (coord.y == cells.size())
    {
        up_down_w_cache = 0;
    }
    else if (coord.x > up_down_w_cache)
    {
        up_down_w_cache = coord.x;
    }
    else if (up_down_w_cache > 0)
    {
        coord.x = up_down_w_cache;
    }

    cursor_pos = get_index_at_coord(coord) + 1;
}


int EditableTextWidget::get_index_at_coord(vector2d coord)
{
    try
    {
        int index = coord_index_map.at(coord);
        return index;
    }
    catch (const std::out_of_range& oor)
    {
        if (coord.x > -1)
        {
            coord.x--;
            return get_index_at_coord(coord);
        }
        else if (coord.y > -1)
        {
            coord.x = size.x;
            coord.y--;
            return get_index_at_coord(coord);
        }
        else
        {
            return -1;
        }
    }
}


vector2d EditableTextWidget::get_coord_at_index(int index)
{
    try
    {
        vector2d coord = index_coord_map.at(index);
        return coord;
    }
    catch (const std::out_of_range& oor)
    {
        return vector2d(-1, 0);
    }
}


void EditableTextWidget::update_cursor_coord()
{
    cursor_coord = get_coord_at_index(cursor_pos - 1);
    cursor_coord += get_absolute_offset();
    cursor_coord.x++;
}


void EditableTextWidget::receive_left_click(vector2d coord, TermWidget* clicked, TermWidget* source)
{
    cursor_pos = get_index_at_coord(coord - get_absolute_offset());
    WIDGET_MAN.draw_widgets();
}

