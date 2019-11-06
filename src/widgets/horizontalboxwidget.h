/**
 * Comfy
 *
 *  Copyright 2019 by Wolfish <wolfish@airmail.cc>
 *  https://wolfish.neocities.org/soft/comfy/
 *
 *  Licensed under the GPL v2.0 only.
 */
#pragma once
#include "multichildwidget.h"


class HorizontalBoxWidget : public MultiChildWidget
{

public:

    HorizontalBoxWidget(vector2d _offset = vector2d(), vector4d _padding = vector4d(), vector4d _child_padding = vector4d());

    virtual void rebuild(bool b_rebuild_children = true) override;

    virtual void set_managed_sizing(std::shared_ptr<TermWidget> wgt) override;
    virtual vector2d get_child_widget_size() const override;
    virtual vector2d size_on_screen() const override;

};

