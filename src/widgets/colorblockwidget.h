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


class ColorBlockWidget : public TermWidget
{

public:

    ColorBlockWidget(uint16_t color = 0, bool _b_fullscreen = true, vector2d _offset = vector2d(), vector4d _padding = vector4d(), vector2d _size = vector2d());

    virtual void rebuild(bool b_rebuild_children = true) override;

};

