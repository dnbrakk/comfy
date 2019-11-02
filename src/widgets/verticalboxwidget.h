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


class VerticalBoxWidget : public TermWidget
{

public:

    VerticalBoxWidget(bool _b_stretch_offscreen = true, vector2d _offset = vector2d(), vector4d _padding = vector4d());

    virtual void rebuild(bool b_rebuild_children = true) override;
    virtual void child_widget_size_change_event() override;
    void add_child_widget(std::shared_ptr<TermWidget> child_widget, bool b_rebuild);
    virtual void draw_children(vector4d constraint = vector4d(-1)) const override;

    virtual vector2d get_child_widget_size() const override;
    virtual void update_child_size(bool b_recursive = false) override;
    virtual TermWidget* get_topmost_child_at(vector2d coord) override;

    virtual vector2d size_on_screen() const override;

    int num_children() const;

    std::vector<std::shared_ptr<TermWidget>> children;

protected:


    bool b_stretch_offscreen;

};

