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

class ImageWidget;
class ColorBlockWidget;


class FSImageWidget : public TermWidget
{

public:

    FSImageWidget(img_packet& _img_pac);


protected:

    img_packet img_pac;
    std::shared_ptr<ImageWidget> image;
    std::shared_ptr<ColorBlockWidget> bg_fill;

    
public:

    virtual void draw(vector4d constraint = vector4d(-1), bool b_draw_children = true) const override;

    virtual void on_focus_lost() override;

    std::shared_ptr<TermWidget> get_child_widget() { return child_widget; };

    virtual void rebuild(bool b_rebuild_children = true) override;

    virtual TermWidget* get_topmost_child_at(vector2d coord) override { return this; };
    virtual void receive_left_click(vector2d coord, TermWidget* clicked = nullptr, TermWidget* source = nullptr) override;

};

