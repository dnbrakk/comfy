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

struct data_4chan;

class ChanWidget : public TermWidget
{

public:

    ChanWidget(vector2d _offset = vector2d(), vector4d _padding = vector4d(), vector4d _child_padding = vector4d(), uint32_t _bg_color = TB_BLACK, uint32_t _fg_color = TB_WHITE,  bool _b_fullscreen = false);

    virtual bool update(data_4chan& chan_data);
    // returns true if redraw is required
    virtual bool on_received_update(data_4chan& chan_data) { return false; };


protected:

    std::shared_ptr<imageboard::page_data> page_data;
    long last_update_time;

};

