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


class BoxWidget : public TermWidget
{

public:

    BoxWidget(vector2d _offset = vector2d(), vector4d _padding = vector4d(), vector2d _size = vector2d(), uint32_t _bg_color = 0, uint32_t _fg_color = 15, bool _b_fullscreen = false);


protected:

    bool b_draw_border;
    // alt colors can be set beginning at coord x,y.
    // e.g., if the box is 10 x 10 term cells,
    // and alt_color_start.x == 2 and
    //     alt_color_start.y == 6, then:
    //
    //      |--------------|
    //      |              |
    //      |              |
    //      |  ------------|
    //      |  | alt color |
    //      |--------------|
    //
    bool b_use_alt_color;
    vector2d alt_color_start;
    uint32_t alt_bg_color;
    uint32_t alt_fg_color;


public:

    void set_use_alt_color(bool b_set) { b_use_alt_color = b_set; };
    void set_alt_color_start(vector2d start) { alt_color_start = start; };
    void set_alt_color_bg(uint32_t alt) { alt_bg_color = alt; };
    void set_alt_color_fg(uint32_t alt) { alt_fg_color = alt; };

    void set_draw_border(bool b_set) { b_draw_border = b_set; };

    virtual void rebuild(bool b_rebuild_children = true) override;
    virtual void child_widget_size_change_event() override;

    virtual vector2d get_child_widget_size() const override;
    virtual vector4d get_child_widget_padding() const override;
    virtual void update_child_size(bool b_recursive = false) override;

};

