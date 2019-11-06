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


class MultiChildWidget : public TermWidget
{

public:

    MultiChildWidget(vector2d _offset = vector2d(), vector4d _padding = vector4d(), vector4d _child_padding = vector4d(), uint32_t _bg_color = TB_BLACK, uint32_t _fg_color = TB_WHITE,  bool _b_fullscreen = false);

    virtual void child_widget_size_change_event() override;
    virtual void set_managed_sizing(std::shared_ptr<TermWidget> wgt) {};
    void add_child_widget(std::shared_ptr<TermWidget> child_widget, bool b_rebuild);
    virtual void draw_children(vector4d constraint = vector4d(-1)) const override;
    virtual void update_child_size(bool b_recursive = false) override;
    virtual TermWidget* get_topmost_child_at(vector2d coord) override;

    std::vector<std::shared_ptr<TermWidget>> children;
    int num_children() const;

};

