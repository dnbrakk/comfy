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

class BoxWidget;
class VerticalBoxWidget;
class TextWidget;
class SelectionWidget;
class BoardsList4chanWidget;


class HomescreenWidget : public ChanWidget
{

public:

    HomescreenWidget(vector2d _offset = vector2d(), vector4d _padding = vector4d(), vector2d _size = vector2d(), uint32_t _bg_color = 0, uint32_t _fg_color = 15, bool _b_fullscreen = false);


protected:

    std::shared_ptr<BoxWidget> main_box;
    std::shared_ptr<BoxWidget> content_box;
    std::shared_ptr<VerticalBoxWidget> vbox;
    std::shared_ptr<TextWidget> logo_text;
    std::shared_ptr<TextWidget> und_text;
    std::shared_ptr<SelectionWidget> main_sel;
    bool b_anim_finished;
    int anim_index;
    int anim_pass;


public:

    virtual bool update(data_4chan& chan_data) override;
    std::shared_ptr<BoardsList4chanWidget> boards_list;

    std::shared_ptr<BoxWidget> get_content_box() { return content_box; };
    virtual void tick_event(std::chrono::milliseconds delta) override;

    static void show_4chan_boards_list(HomescreenWidget* hs);
    static void show_saved_threads(HomescreenWidget* hs);
    static void show_about_info(HomescreenWidget* hs);
    static void show_help(HomescreenWidget* hs);
    static void quit_application();

    virtual void rebuild(bool b_rebuild_children = true) override;
    virtual void handle_term_resize_event() override;
    virtual bool handle_key_input(const tb_event& input_event, bool b_bubble_up = true) override;
    virtual void child_widget_size_change_event() override;

    virtual vector2d get_child_widget_size() const override;
    virtual vector4d get_child_widget_padding() const override;
    virtual void update_child_size(bool b_recursive = false) override;

};

