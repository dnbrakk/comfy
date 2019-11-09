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

        index++;
        width++;

        // look ahead to see if word wrap is needed
        if (ch == L' ')
        {
            for (int j = i + 1; j < buf.length(); ++j)
            {
                const wchar_t& ch2 = buf[j];
                if (ch2 == L' ' || ch2 == L'\n')
                    break;

                // next word exceeds max width, so wrap
                if (j - index + width >= max_width)
                {
                    push_row(row);
                    width = 0;
                    index_coord_map[index] = vector2d(width, cells.size());
                    break;
                }
            }
        }
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
        return;

    vector2d curr_coord = get_coord_at_index(cursor_pos - 1);
    if (curr_coord.y == 0 || (curr_coord.y == 1 && curr_coord.x == -1))
    {
        cursor_pos = 0;
        return;
    }

    int last_in_row = 0;

    for (int i = 0; i < buf.length(); ++i)
    {
        const vector2d& coord = get_coord_at_index(i);
        if (coord.y == curr_coord.y - 1)
        {
            last_in_row = i;
            // row above is longer than or same length
            // as row cursor is on
            if (coord.x == curr_coord.x)
            {
                int index = i;
                int y = coord.y;
                int x = coord.x;
                // set index to up down cache if applicable
                for (int j = i + 1; j < buf.length(); ++j)
                {
                    coord = get_coord_at_index(j);
                    if (coord.y > y || coord.x == 0)
                    {
                        break;
                    }
                    else
                    {
                        x = coord.x;
                        if (x > up_down_w_cache)
                            break;

                        index = j;
                    }
                }

                cursor_pos = index + 1;
                return;
            }
        }
        // row above is shorter than row cursor is on
        else if (coord.y == curr_coord.y)
        {
            if (curr_coord.x > up_down_w_cache)
                up_down_w_cache = curr_coord.x;

            cursor_pos = last_in_row + 1;
            return;
        }
    }
}


void EditableTextWidget::move_cursor_down()
{
    if (cursor_pos == buf.length())
        return;

    vector2d curr_coord = get_coord_at_index(cursor_pos - 1);
    if (cursor_pos == 0)
        curr_coord = vector2d(-1, 0);

    int last_in_row = 0;

    for (int i = 0; i < buf.length(); ++i)
    {
        const vector2d& coord = get_coord_at_index(i);
        if (coord.y == curr_coord.y + 1)
        {
            last_in_row = i;
            // row below is longer than or same length
            // as row cursor is on
            if (coord.x == curr_coord.x || curr_coord.x < 0)
            {
                int index = i;
                int y = coord.y;
                int x = coord.x;
                // set index to up down cache if applicable
                for (int j = i + 1; j < buf.length(); ++j)
                {
                    coord = get_coord_at_index(j);
                    if (coord.y > y || coord.x == 0)
                    {
                        break;
                    }
                    else
                    {
                        x = coord.x;
                        if (x > up_down_w_cache)
                            break;

                        index = j;
                    }
                }

                cursor_pos = index + 1;
                return;
            }
        }
        // last row
        else if (curr_coord.y == cells.size() - 1)
        {
            last_in_row = i;
        }
    }

    if (curr_coord.x > up_down_w_cache)
        up_down_w_cache = curr_coord.x;

    // row below is shorter than row cursor is on
    cursor_pos = last_in_row + 1;
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
    // if word has been wrapped
    if (cursor_pos < buf.length())
    {
        vector2d adj = get_coord_at_index(cursor_pos);
        const wchar_t& ch = buf[cursor_pos];
        if (adj.y > cursor_coord.y && ch != L'\n')
        {
            cursor_coord = vector2d(-1, adj.y);
        }
    }
    cursor_coord += get_absolute_offset();
    cursor_coord.x++;
}


void EditableTextWidget::receive_left_click(vector2d coord, TermWidget* clicked, TermWidget* source)
{
}

