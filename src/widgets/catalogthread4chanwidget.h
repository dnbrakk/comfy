/**
 * Comfy
 *
 *  Copyright 2019 by Wolfish <wolfish@airmail.cc>
 *  https://wolfish.neocities.org/soft/comfy/
 *
 *  Licensed under the GPL v2.0 only.
 */
#pragma once
#include "post4chanwidget.h"

class Catalog4chanWidget;
class TextWidget;
class BoxWidget;
class BoxDividerWidget;
class VerticalBoxWidget;


class CatalogThread4chanWidget : public Post4chanWidget
{

public:

    CatalogThread4chanWidget(Catalog4chanWidget* _catalog, imageboard::post& _post_data, vector4d _padding = vector4d(), uint32_t _bg_color = COLO.post_bg, uint32_t _fg_color = COLO.post_text);


protected:

    Catalog4chanWidget* catalog;
    vector2d img_size;
    int reply_count;
    int image_count;

    std::shared_ptr<TextWidget> post_title;

    virtual void rebuild_vbox() override;


public:

    // returns true if there was any change to the counts
    bool update_reply_and_image_count(imageboard::post& _post_data);

    virtual void select() override;
    virtual void unselect() override;

    virtual bool add_image(img_packet& pac, bool b_refresh_parent = true) override;

    virtual void rebuild(bool b_rebuild_children = true) override;

    virtual void receive_left_click(vector2d coord, TermWidget* clicked = nullptr, TermWidget* source = nullptr) override;

};

