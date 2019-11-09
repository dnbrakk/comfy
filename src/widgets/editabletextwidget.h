/**
 * Comfy
 *
 *  Copyright 2019 by Wolfish <wolfish@airmail.cc>
 *  https://wolfish.neocities.org/soft/comfy/
 *
 *  Licensed under the GPL v2.0 only.
 */
#pragma once
#include "termwidget.h"


class EditableTextWidget : public TermWidget
{

public:

    EditableTextWidget(vector2d _offset = vector2d(), vector4d _padding = vector4d(), uint32_t _bg_color = 16, uint32_t _fg_color = 255);


protected:

    // maps buffer index to cell coordinates
    std::map<int, vector2d> index_coord_map;
    std::wstring buf;
    int widest_row;
    int cursor_pos;
    // used when moving the cursor up and down
    int up_down_w_cache;

    // returns true if max height has been reached
    void push_row(std::vector<tb_cell>& row);
    void make_cells(std::vector<tb_cell>& row, const std::wstring& text);


public:

    const std::wstring& get_buffer() const { return buf; };
    int get_cursor_pos() const { return cursor_pos; };
    vector2d get_coord_at_index(int index);

    virtual void rebuild(bool b_rebuild_children = true) override;
    virtual void update_cursor_coord() override;

    void insert_char(wchar_t ch, bool b_rebuild = true);
    void remove_char(bool b_rebuild = true);

    void move_cursor_left();
    void move_cursor_right();
    void move_cursor_up();
    void move_cursor_down();

    void clear_text() {
        buf.clear();
        index_coord_map.clear();
        cursor_pos = 0;
        widest_row = 0;
        up_down_w_cache = 0;
    };

    virtual void receive_left_click(vector2d coord, TermWidget* clicked = nullptr, TermWidget* source = nullptr) override;

};

