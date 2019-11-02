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

class ScrollPanelWidget;


class ScrollBarWidget : public TermWidget
{

public:

    ScrollBarWidget(ScrollPanelWidget* _parent_panel, uint32_t _bg_color = 15, uint32_t _fg_color = 0);

    virtual void rebuild(bool b_rebuild_children = true) override;


protected:

    int doc_size;
    int doc_position;

    ScrollPanelWidget* parent_panel;

};

