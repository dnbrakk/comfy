/**
 * Comfy
 *
 *  Copyright 2019 by Wolfish <wolfish@airmail.cc>
 *  https://wolfish.neocities.org/soft/comfy/
 *
 *  Licensed under the GPL v2.0 only.
 */
#include "fsimagewidget.h"
#include "../widgetman.h"
#include "widgets.h"


FSImageWidget::FSImageWidget(img_packet& _img_pac)
: TermWidget(vector2d(), vector4d(), vector4d(), 0, 0, false)
, img_pac(_img_pac)
{
    set_h_sizing(e_widget_sizing::ws_fullscreen);
    set_v_sizing(e_widget_sizing::ws_fullscreen);

    set_can_switch_to(false);

    image = std::make_shared<ImageWidget>(
        img_pac,
        true, // maintain aspect ratio
        false, // fullscreen on click
        vector2d(1, 1) // size
    );
    image->set_h_align(e_widget_align::wa_center);
    image->set_v_align(e_widget_align::wa_top);
    image->set_h_sizing(e_widget_sizing::ws_fill);
    image->set_v_sizing(e_widget_sizing::ws_fill);

    bg_fill = std::make_shared<ColorBlockWidget>(COLO.fs_image_bg, true);

    child_widget = image;
    rebuild();
}


void FSImageWidget::rebuild(bool b_rebuild_children)
{
    if (child_widget)
    {
        child_widget->rebuild(true);
    }
}


void FSImageWidget::draw(vector4d constraint, bool b_draw_children) const
{
    if (b_hidden) return;

    if (bg_fill)
    {
        bg_fill->draw();
    }

    if (child_widget)
    {
        child_widget->draw(constraint);
    }
}


void FSImageWidget::on_focus_lost()
{
    delete_self();
}


void FSImageWidget::receive_left_click(vector2d coord, TermWidget* clicked, TermWidget* source)
{
    delete_self();
    WIDGET_MAN.termbox_clear(COLO.img_artifact_remove);
    WIDGET_MAN.termbox_draw();
    WIDGET_MAN.draw_widgets();
}

