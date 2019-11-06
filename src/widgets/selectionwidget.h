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
#include <functional>

class BoxWidget;
class VerticalBoxWidget;
class ScrollPanelWidget;


class SelectionWidget : public TermWidget
{

public:

    SelectionWidget(uint32_t _bg_color = COLO.selection_bg, uint32_t _fg_color = COLO.selection_fg, uint32_t _border_color = COLO.selection_fg, vector2d _offset = vector2d(), vector4d _padding = vector4d(), vector2d _size = vector2d(), bool _b_fullscreen = false);


protected:

    uint32_t border_color;
    bool b_draw_border;
    std::shared_ptr<BoxWidget> main_box;
    std::shared_ptr<VerticalBoxWidget> vbox;
    std::shared_ptr<ScrollPanelWidget> scroll_panel;
    int selection;
    std::vector<std::string> selection_names;
    std::vector<std::function<void()>> selection_fns;

    void activate_selection();

    vector2d scroll_offset;
    // max length of selection string
    int max_sel_len;


public:

    int get_selection() const { return selection; };
    void set_selection(int set);
    int get_num_selections() const { return selection_names.size(); };
    void set_draw_border(bool b_set) { b_draw_border = b_set; };

    virtual bool handle_key_input(const tb_event& input_event, bool b_bubble_up = true) override;
    bool handle_key_input(const tb_event& input_event);
    virtual void handle_term_resize_event() override;

    void add_selection(std::string name, std::function<void()> fn);

    virtual void rebuild(bool b_rebuild_children = true) override;

    virtual vector2d get_child_widget_size() const override;
    virtual vector4d get_child_widget_padding() const override;
    virtual void update_child_size(bool b_recursive = false) override;

    virtual void on_focus_received() override;
    virtual void on_focus_lost() override;

    virtual void receive_left_click(vector2d coord, TermWidget* clicked = nullptr, TermWidget* source = nullptr) override;

};

