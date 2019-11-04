/*
 * Comfy
 *
 *  Copyright 2019 by Wolfish <wolfish@airmail.cc>
 *  https://wolfish.neocities.org/soft/comfy/
 *
 *  Licensed under the GPL v2.0 only.
 */
#pragma once
#include "termwidget.h"

class ScrollBarWidget;


class ScrollPanelWidget : public TermWidget
{

public:

    ScrollPanelWidget(bool _b_has_scrollbar = false, vector2d _offset = vector2d(), bool _b_allow_vert_scroll = true, bool _b_allow_horiz_scroll = false, uint32_t _bg_color = 0, uint32_t _fg_color = 15);


protected:

    std::shared_ptr<ScrollBarWidget> scroll_bar_widget;

    bool b_has_scrollbar;
    bool b_allow_vert_scroll;
    bool b_allow_horiz_scroll;

    void scroll_pos_sanity_check();
    void update_scroll_bar();


public:

    void scroll(int dist);
    void scroll_to(int pos);
    void scroll_to_end();
    void scroll_to_beginning();

    // the dimensions of the panel that are visible in term
    int get_visible_height();
    int get_visible_width();

    vector2d get_scroll_position() const { return offset; };
    void set_scroll_position(vector2d pos) { offset = pos; };

    virtual bool handle_key_input(const tb_event& input_event, bool b_bubble_up = true) override;

    virtual void rebuild(bool b_rebuild_children = true) override;
    virtual void child_widget_size_change_event() override;

    virtual vector2d get_child_widget_size() const override;
    virtual void update_child_size(bool b_recursive = false) override;

    vector4d get_draw_constraint();
    virtual void draw_children(vector4d constraint = vector4d(-1)) const override;
};

