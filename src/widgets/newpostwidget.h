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

class BoxWidget;
class ScrollPanelWidget;
class TextWidget;


class NewPostWidget : public TermWidget
{

public:

    NewPostWidget(vector2d _offset = vector2d(), vector4d _padding = vector4d(), vector2d _size = vector2d(), uint32_t _bg_color = 0, uint32_t _fg_color = 15, bool _b_fullscreen = false);


protected:

    std::shared_ptr<BoxWidget> main_box;
    std::shared_ptr<ScrollPanelWidget> scroll_panel;
    std::shared_ptr<TextWidget> post_text;
    std::wstring post_text_buf;
    int cursor_pos;


public:

    virtual void rebuild(bool b_rebuild_children = true) override;
    virtual void child_widget_size_change_event() override;

    virtual vector2d get_child_widget_size() const override;
    virtual vector4d get_child_widget_padding() const override;
    virtual void update_child_size(bool b_recursive = false) override;

    virtual bool handle_key_input(const tb_event& input_event, bool b_bubble_up = true) override;

};

