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


public:

    void set_draw_border(bool b_set) { b_draw_border = b_set; };

    void set_child_widget(std::shared_ptr<TermWidget> _child_widget, bool b_rebuild = true);
    std::shared_ptr<TermWidget> get_child_widget() { return child_widget; };

    virtual void rebuild(bool b_rebuild_children = true) override;
    virtual void child_widget_size_change_event() override;

    virtual vector2d get_child_widget_size() const override;
    virtual vector4d get_child_widget_padding() const override;
    virtual void update_child_size(bool b_recursive = false) override;

};

