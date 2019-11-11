/**
 * Comfy
 *
 *  Copyright 2019 by Wolfish <wolfish@airmail.cc>
 *  https://wolfish.neocities.org/soft/comfy/
 *
 *  Licensed under the GPL v2.0 only.
 */
#include "imagewidget.h"
#include "../widgetman.h"
#include "fsimagewidget.h"


ImageWidget::ImageWidget(img_packet& _img_pac, bool _b_maintain_ar, bool _b_fullscreen_on_click, vector2d _size, vector2d _offset, vector4d _padding)
: TermWidget(_offset, _padding, vector4d(), 15 /* white */, 15, false)
, img_pac(_img_pac)
, b_maintain_ar(_b_maintain_ar)
, b_fullscreen_on_click(_b_fullscreen_on_click)
, original_size(_size)
{
    if (DISPLAY_IMAGES)
    {
        // ensure that img_data for this image persists for the
        // lifetime of this widget, and when this widget destructs,
        // if this is the last checkout_token for the img_data that
        // exists, the img_data is removed from the map and the
        // associated data garbage collected
        img_token = IMG_MAN.checkout_img(img_pac.image_key);

        set_size(_size);
        set_h_sizing(e_widget_sizing::ws_fill);
        set_v_sizing(e_widget_sizing::ws_fill);
        img_size_pix = IMG_MAN.get_img_size(img_pac.image_key);
        last_size_drawn = vector2d(-1);
        last_offset_drawn = vector2d(-1);
    }
}


ImageWidget::~ImageWidget()
{
}


void ImageWidget::rebuild(bool b_rebuild_children)
{
    if (!DISPLAY_IMAGES) return;

    int w = get_width_constraint();
    int h = get_height_constraint();

    // fixed size may have been changed by auto sizing;
    // use original size for calculations
    if (h_sizing == ws_fixed) w = original_size.x;
    if (v_sizing == ws_fixed) h = original_size.y;

    // maintain aspect ratio
    if (b_maintain_ar)
    {
        // slave one dimension to the fixed size of other other dimension
        if ((h_sizing == ws_dynamic && v_sizing == ws_fixed))
        {
            w = -1;
        }
        else if (v_sizing == ws_dynamic && h_sizing == ws_fixed)
        {
            h = -1;
        }
        // choose dimension based on container constraint
        else
        {
            vector2d char_size = IMG_MAN.get_term_char_size();
            float img_ar = (float)img_size_pix.x / (float)img_size_pix.y;
            float container_ar = (float)(w * char_size.x) / (float)(h * char_size.y);
            if (container_ar > img_ar)
            {
                w = -1;
            }
            else
            {
                h = -1;
            }
        }
    }

    vector2d new_size = IMG_MAN.calc_img_size_term_chars(img_size_pix, w, h);

    // prevent image overflow when auto slaving to one dimension
    if ((h_sizing == ws_dynamic && v_sizing == ws_fixed) &&
         new_size.x > get_max_parent_width())
    {
        w = get_max_parent_width();
        h = -1;
        new_size = IMG_MAN.calc_img_size_term_chars(img_size_pix, w, h);
    }
    else if ((v_sizing == ws_dynamic && h_sizing == ws_fixed) &&
              new_size.y > get_max_parent_height())
    {
        h = get_max_parent_height();
        w = -1;
        new_size = IMG_MAN.calc_img_size_term_chars(img_size_pix, w, h);
    }

    set_size(new_size);
}


void ImageWidget::blast_out_image_artifacts_at_last_position()
{
    if (last_size_drawn.x < 0 || last_size_drawn.y < 0 ||
        last_offset_drawn.x < 0 || last_offset_drawn.y < 0)
    {
        return;
    }

    for (int y = last_offset_drawn.y;
         y < last_offset_drawn.y + last_size_drawn.y;
         ++y)
    {
        for (int x = last_offset_drawn.x;
             x < last_offset_drawn.x + last_size_drawn.x;
             ++x)
        {
            WIDGET_MAN.remove_image_artifact(vector2d(x, y));
        }
    }
}


void ImageWidget::draw(vector4d constraint, bool b_draw_children) const
{
    if (!DISPLAY_IMAGES || b_hidden) return;

    // x start
    if (constraint.a == -1) constraint.a = 0;
    // x end
    if (constraint.b == -1) constraint.b = term_w();
    // y start
    if (constraint.c == -1) constraint.c = 0;
    // y end
    if (constraint.d == -1) constraint.d = term_h();

    int w = get_width_constraint();
    int h = get_height_constraint();

    vector2d absolute_offset = get_absolute_offset();
    img_offset_cache = absolute_offset;

    // do not draw if off screen
    if (absolute_offset.x < -w || absolute_offset.x > term_w() ||
        absolute_offset.y < -h || absolute_offset.y > term_h())
    {
        // blast out image artifacts
        if (!last_size_drawn.is_zero())
        {
            for (int y = last_offset_drawn.y;
                 y < last_offset_drawn.y + last_size_drawn.y;
                 ++y) {

                for (int x = last_offset_drawn.x;
                     x < last_offset_drawn.x + last_size_drawn.x;
                     ++x) {

                    WIDGET_MAN.remove_image_artifact(vector2d(x, y));
                }
            }
        }

        last_size_drawn = vector2d(0);
        return;
    }

    // fixed size may have been changed by auto sizing;
    // use original size for calculations
    if (h_sizing == ws_fixed) w = original_size.x;
    if (v_sizing == ws_fixed) h = original_size.y;

    // maintain aspect ratio
    if (b_maintain_ar)
    {
        // slave one dimension to the fixed size of other dimension
        if ((h_sizing == ws_dynamic && v_sizing == ws_fixed))
        {
            w = -1;
        }
        else if (v_sizing == ws_dynamic && h_sizing == ws_fixed)
        {
            h = -1;
        }
        // choose dimension based on container constraint
        else
        {
            vector2d char_size = IMG_MAN.get_term_char_size();
            float img_ar = (float)img_size_pix.x / (float)img_size_pix.y;
            float container_ar = (float)(w * char_size.x) / (float)(h * char_size.y);
            if (container_ar > img_ar)
            {
                w = -1;
            }
            else
            {
                h = -1;
            }
        }
    }

    // prevent image overflow when auto slaving to one dimension
    vector2d new_size = IMG_MAN.calc_img_size_term_chars(img_size_pix, w, h);

    if ((h_sizing == ws_dynamic && v_sizing == ws_fixed) &&
         new_size.x > get_max_parent_width())
    {
        w = get_max_parent_width();
        h = -1;
    }
    else if ((v_sizing == ws_dynamic && h_sizing == ws_fixed) &&
              new_size.y > get_max_parent_height())
    {
        h = get_max_parent_height();
        w = -1;
    }

    // auto cropping
    int left = 0;
    int top = 0;
    int right = 0;
    int bottom = 0;

    // need to recalc size as w or h could be == -1 at this point
    new_size = IMG_MAN.calc_img_size_term_chars(img_size_pix, w, h);

    if (absolute_offset.x < constraint.a)
        left = constraint.a - absolute_offset.x;

    if (absolute_offset.y < constraint.c)
        top = constraint.c - absolute_offset.y;

    if (absolute_offset.x + new_size.x > constraint.b)
        right = (absolute_offset.x + new_size.x) - constraint.b;

    if (absolute_offset.y + new_size.y > constraint.d)
        bottom = (absolute_offset.y + new_size.y) - constraint.d;

    last_crop = vector4d(left, top, right, bottom);
    vector2d offset_drawn = vector2d(
        absolute_offset.x + last_crop.a,
        absolute_offset.y + last_crop.b);

    // draw to buffer
    vector2d size_drawn = IMG_MAN.draw_img_term_coord_and_size(
        img_pac.image_key,
        w, h,
        offset_drawn.x,
        offset_drawn.y,
        last_crop
    );

    // blast out image artifacts
    for (int y = last_offset_drawn.y;
         y < last_offset_drawn.y + last_size_drawn.y;
         ++y) {

        for (int x = last_offset_drawn.x;
             x < last_offset_drawn.x + last_size_drawn.x;
             ++x) {

            if (x < offset_drawn.x || x >= size_drawn.x + offset_drawn.x ||
                y < offset_drawn.y || y >= size_drawn.y + offset_drawn.y) {

                WIDGET_MAN.remove_image_artifact(vector2d(x, y));
            }
        }
    }

    last_size_drawn = size_drawn;
    last_offset_drawn = offset_drawn;

    if (size_drawn.is_zero())
        return;

    // prevent termbox from writing cells to where image is drawn
    for (int y = 0; y < last_size_drawn.y; ++y)
    {
        for (int x = 0; x < last_size_drawn.x; ++x)
        {
            //if (last_size_drawn.y == 1) ERR(y + last_offset_drawn.y, "o: ");
            //ERR(x + last_offset_drawn.x, "x: ");
            //ERR(y + last_offset_drawn.y, "x: ");

            WIDGET_MAN.add_null_cell(
                vector2d(x + last_offset_drawn.x, y + last_offset_drawn.y)
            );
        }
    }

    // size must be set to full image size without crop
    // so that the container box does not shrink and
    // screw up the layout
    set_size(last_size_drawn.x + last_crop.a + last_crop.c,
             last_size_drawn.y + last_crop.b + last_crop.d);
}


void ImageWidget::size_change_event()
{
    // make sure box follows size of image
    if (parent_widget)
    {
        parent_widget->update_size(false);
        parent_widget->rebuild(false /* recursive */);
    }
}


void ImageWidget::receive_left_click(vector2d coord, TermWidget* clicked, TermWidget* source)
{
    if (b_fullscreen_on_click)
    {
        std::shared_ptr<FSImageWidget> fsimage =
            std::make_shared<FSImageWidget>(img_pac);
        std::string img_id = img_pac.image_key;
        img_id += "_fullscreen";
        fsimage->set_id(img_id);
        WIDGET_MAN.add_widget(fsimage);
        WIDGET_MAN.draw_widgets();
    }
    else
    {
        TermWidget::receive_left_click(coord, clicked, this);
    }
}

