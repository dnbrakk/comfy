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

class Thread4chanWidget;
class TextWidget;
class BoxWidget;
class BoxDividerWidget;
class VerticalBoxWidget;
class HorizontalBoxWidget;
class ImageWidget;


class Post4chanWidget : public TermWidget
{

public:

    Post4chanWidget(Thread4chanWidget* _thread, imageboard::post& _post_data, vector4d _padding = vector4d(), uint32_t _bg_color = COLO.post_bg, uint32_t _fg_color = COLO.post_text);


protected:

    bool b_is_op;
    int post_num;
    Thread4chanWidget* thread;
    imageboard::post* post_data;

    std::shared_ptr<BoxWidget> main_box;
    std::shared_ptr<VerticalBoxWidget> vbox;
    std::shared_ptr<HorizontalBoxWidget> info_hbox;
    std::shared_ptr<BoxWidget> flag_box;
    std::shared_ptr<BoxWidget> image_box;
    std::shared_ptr<BoxWidget> post_info_box;
    std::shared_ptr<TextWidget> post_info;
    std::shared_ptr<TextWidget> img_info;
    std::shared_ptr<TextWidget> post_text;
    std::shared_ptr<BoxDividerWidget> reply_div;
    std::shared_ptr<TextWidget> replies_text;
    std::vector<int> replies;

    virtual void rebuild_vbox();

    bool b_selected;


public:

    virtual std::vector<ImageWidget*> get_image_widgets();

    virtual void select();
    virtual void unselect();

    bool is_op() const { return b_is_op; };
    int get_post_num() const { return post_num; };

    void add_reply(int reply) { replies.push_back(reply); };
    // searches post text for post numbers (e.g. >>9398223)
    // and notifies thread, adding this post's id to the replies
    // vector of the post this post is quoting
    void parse_quotes();
    void load_replies();

    virtual bool add_image(img_packet& pac, bool b_refresh_parent = true);

    virtual void rebuild(bool b_rebuild_children = true) override;

    virtual vector2d get_child_widget_size() const override;
    virtual void update_child_size(bool b_recursive = false) override;

    virtual void receive_left_click(vector2d coord, TermWidget* clicked = nullptr, TermWidget* source = nullptr) override;

};

