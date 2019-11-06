/**
 * Comfy
 *
 *  Copyright 2019 by Wolfish <wolfish@airmail.cc>
 *  https://wolfish.neocities.org/soft/comfy/
 *
 *  Licensed under the GPL v2.0 only.
 */
#include "chanwidget.h"
#include "../netops.h"

ChanWidget::ChanWidget(vector2d _offset, vector4d _padding, vector4d _child_padding, uint32_t _bg_color, uint32_t _fg_color, bool _b_fullscreen)
: TermWidget(_offset, _padding, _child_padding, _bg_color, _fg_color, _b_fullscreen)
{
    page_data = nullptr;
    last_update_time = 0;
}


// returns true if there are changes and redraw is required
bool ChanWidget::update(data_4chan& chan_data)
{
    bool b_redraw = on_received_update(chan_data);
    last_update_time = chan_data.fetch_time;

    // error
    if (!chan_data.is_valid())
    {
        std::string err = "Error: chan_data invalid.\n";

        switch(chan_data.error_type)
        {
            case et_invalid_url         : err += "Invalid URL.\n";
                                          break;

            case et_http_404            : err += "404 not found.\n";
                                          break;

            case et_json_parse          : err += "JSON parse error.\n";
                                          break;

            case et_not_mod_since       : return;
        }

        err += "CURL error code: " + std::to_string(chan_data.curl_result);
        err += "\nHTTP response: " + std::to_string(chan_data.http_response);
        ERR(err);
    }
    else
    {
        page_data = chan_data.page_data;
        rebuild();

        b_redraw = true;
    }

    return b_redraw;
}

