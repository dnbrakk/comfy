/**
 * Comfy
 *
 *  Copyright 2019 by Wolfish <wolfish@airmail.cc>
 *  https://wolfish.neocities.org/soft/comfy/
 *
 *  Licensed under the GPL v2.0 only.
 */
#pragma once
#include "chanwidget.h"

struct data_4chan;
class ColorBlockWidget;
class SelectionWidget;


class BoardsList4chanWidget : public ChanWidget
{

public:

    BoardsList4chanWidget(data_4chan& chan_data);


protected:

    std::shared_ptr<SelectionWidget> board_selections;


public:

    static void open_board(std::string url);

    virtual bool handle_key_input(const tb_event& input_event, bool b_bubble_up = true) override;
    virtual void handle_term_resize_event() override;
    virtual void on_focus_received();

    virtual void refresh(bool b_draw_term = true) override;
    virtual void rebuild(bool b_rebuild_children = true) override;
    virtual void child_widget_size_change_event() override;

    virtual void draw_children(vector4d constraint = vector4d(-1)) const override;

    virtual vector2d get_child_widget_size() const override;
    virtual void update_child_size(bool b_recursive = false) override;
};

