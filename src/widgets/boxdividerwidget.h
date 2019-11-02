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


class BoxDividerWidget : public TermWidget
{

public:

    BoxDividerWidget(bool _b_horizontal = true, uint32_t _bg_color = 0, uint32_t _fg_color = 15);

    virtual void rebuild(bool b_rebuild_children = true) override;


protected:

    bool b_horizontal;

};

