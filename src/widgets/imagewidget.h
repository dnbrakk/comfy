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
#include "../imgman.h"


class ImageWidget : public TermWidget
{

public:

    ImageWidget(img_packet& _img_pac, bool _b_maintain_ar = true, bool _b_fullscreen_on_click = true, vector2d _size = vector2d(), vector2d _offset = vector2d(), vector4d _padding = vector4d());
    ~ImageWidget();


protected:

    img_packet img_pac;
    // maintain aspect ratio
    bool b_maintain_ar;
    bool b_fullscreen_on_click;

    // size that the image was asked to be displayed
    // at when first created
    vector2d original_size;
    // actual image size in pixels
    vector2d img_size_pix;
    //vector2d img_size_cache;
    vector2d img_offset_cache;
    vector4d last_crop;
    vector2d last_size_drawn;
    vector2d last_offset_drawn;
    bool b_cleared_last;

    std::shared_ptr<checkout_token> img_token;


public:

    // draws image artifact remove term cells over entire
    // area that the image was last drawn
    void blast_out_image_artifacts_at_last_position();

    virtual void draw(vector4d constraint = vector4d(-1), bool b_draw_children = true) const override;

    virtual void size_change_event() override;
    std::shared_ptr<TermWidget> get_child_widget() { return child_widget; };

    virtual void rebuild(bool b_rebuild_children = true) override;

    virtual void receive_left_click(vector2d coord, TermWidget* clicked = nullptr, TermWidget* source = nullptr) override;

};

