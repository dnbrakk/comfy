/**
 * Comfy
 *
 *  Copyright 2019 by Wolfish <wolfish@airmail.cc>
 *  https://wolfish.neocities.org/soft/comfy/
 *
 *  Licensed under the GPL v2.0 only.
 */
#include "selectionwidget.h"
#include "../widgetman.h"
#include "widgets.h"


SelectionWidget::SelectionWidget(uint32_t _bg_color, uint32_t _fg_color, uint32_t _border_color, vector2d _offset, vector4d _padding, vector2d _size, bool _b_fullscreen)
: TermWidget(_offset, _padding, vector4d(), _bg_color, _fg_color, _b_fullscreen)
{
    border_color = _border_color;
    b_draw_border = true;
    selection = 0;
    scroll_offset = vector2d();
    set_h_sizing(ws_fixed);
    set_v_sizing(ws_dynamic);
    set_h_align(wa_center);
    set_v_align(wa_center);

    set_size(_size);
}


bool SelectionWidget::handle_key_input(const tb_event& input_event, bool b_bubble_up)
{
    bool b_handled = false;

    if (!vbox) return false;

    switch(input_event.key)
    {
        case TB_KEY_ARROW_DOWN          :
            if (selection < vbox->num_children() - 1)
            {
                selection++;
                rebuild(true);
                WIDGET_MAN.draw_widgets(
                    this,
                    false,
                    false);
            }
            b_handled = true;
            break;

        case TB_KEY_ARROW_UP            :
            if (selection > 0) 
            {
                selection--;
                rebuild(true);
                WIDGET_MAN.draw_widgets(
                    this,
                    false,
                    false);
            }
            b_handled = true;
            break;

        case TB_KEY_ARROW_LEFT          :
            if (selection - 10 > 0)
            {
                selection -= 10;
            }
            else
            {
                selection = 0;
            }
            rebuild(true);
            WIDGET_MAN.draw_widgets(
                this,
                false,
                false);
            b_handled = true;
            break;

        case TB_KEY_ARROW_RIGHT          :
            if (selection + 10 < vbox->num_children() - 1)
            {
                selection += 10;
            }
            else
            {
                selection = vbox->num_children() -1;
            }
            rebuild(true);
            WIDGET_MAN.draw_widgets(
                this,
                false,
                false);
            b_handled = true;
            break;

        case TB_KEY_TAB                 :
            if (selection < vbox->num_children() - 1)
            {
                selection++;
            }
            else
            {
                selection = 0;
            }
            rebuild(true);
            WIDGET_MAN.draw_widgets(
                this,
                false,
                false);
            b_handled = true;
            break;

        case TB_KEY_ENTER                 :
            activate_selection();
            b_handled = true;
            break;

        case TB_KEY_SPACE                 :
            activate_selection();
            b_handled = true;
            break;
    }

    return b_handled;
}


void SelectionWidget::activate_selection()
{
    if (selection_fns.size() > selection)
    {
        selection_fns[selection]();
    }
}


void SelectionWidget::add_selection(std::string name, std::function<void()> fn)
{
    if (name.empty()) return;

    selection_names.push_back(name);
    selection_fns.push_back(fn);
}


void SelectionWidget::handle_term_resize_event()
{
    rebuild(true);
    WIDGET_MAN.termbox_clear(TB_BLACK);
    WIDGET_MAN.termbox_draw();
    WIDGET_MAN.draw_widgets(this);
}


void SelectionWidget::rebuild(bool b_rebuild_children)
{
    cells.clear();

    vbox = std::make_shared<VerticalBoxWidget>();
    vbox->set_h_sizing(e_widget_sizing::ws_fill);
    vbox->set_v_sizing(e_widget_sizing::ws_fill);

    scroll_panel = 
        std::make_shared<ScrollPanelWidget>(
            false,  // has scroll bar
            vector2d(),
            true,
            false,
            bg_color,
            fg_color);
    scroll_panel->set_h_sizing(e_widget_sizing::ws_fill);
    scroll_panel->set_v_sizing(e_widget_sizing::ws_fill);
    scroll_panel->set_child_widget(vbox, false /* rebuild */);
    scroll_panel->set_scroll_position(scroll_offset);

    main_box = std::make_shared<BoxWidget>(
        vector2d(),
        vector4d(),
        vector2d(),
        bg_color,
        border_color);
    main_box->set_h_sizing(e_widget_sizing::ws_fill);
    main_box->set_v_sizing(e_widget_sizing::ws_fill);
    main_box->set_draw_border(b_draw_border);
    main_box->set_child_widget(scroll_panel, false);

    vector4d pad = main_box->get_child_padding();
    max_sel_len = get_max_parent_width() - pad.a - pad.c;
    int longest_sel = 0;
    for (int i = 0; i < selection_names.size(); ++i)
    {
        uint8_t bg;
        uint8_t fg;
        if (i == selection)
        {
            bg = fg_color;
            fg = bg_color;
        }
        else
        {
            bg = bg_color;
            fg = fg_color;
        }

        std::string sel_s = selection_names[i];
        if (sel_s.length() > max_sel_len)
        {
            sel_s = sel_s.substr(0, max_sel_len - 3);
            sel_s += "...";
        }

        std::shared_ptr<TextWidget> sel =
            std::make_shared<TextWidget>(
                vector2d(),
                vector4d(0, 0, 0, 0),
                bg,
                fg);
        sel->append_text(sel_s);

        if (sel_s.length() > longest_sel)
        {
            longest_sel = sel_s.length();
        }

        vbox->add_child_widget(sel, false);
    }

    int max_w = 0;
    int max_h = 0;
    if (get_h_sizing() == ws_fixed)
    {
        // set width to fit longest selection string
        max_w = longest_sel + pad.a + pad.c;
    }
    else
    {
        max_w = get_width_constraint();
    }

    if (get_v_sizing() == ws_dynamic)
    {
        max_h = get_max_parent_height();
        if (selection_names.size() < max_h)
        {
            vector4d pad = main_box->get_child_padding();
            max_h = selection_names.size() + pad.b + pad.d;
        }
    }
    else
    {
        max_h = get_height_constraint();
    }
    set_size(max_w, max_h);

    set_child_widget(main_box, false /* rebuild */);

    main_box->rebuild(true);

    // scroll to keep selection on screen
    if (selection >= 0 && selection < vbox->num_children())
    {
        std::shared_ptr<TermWidget> c = vbox->children[selection];
        if (c)
        {
            // constraint:
            //     a = x start
            //     b = x end
            //     c = y start
            //     d = y end
            vector4d constraint = scroll_panel->get_draw_constraint();
            vector2d offs = c->get_absolute_offset();

            if (offs.y + scroll_offset.y + 1 > constraint.d)
            {
                scroll_offset.y = -(offs.y - constraint.d + 1);
                scroll_panel->set_scroll_position(scroll_offset);
            }
            else if (offs.y + scroll_offset.y < constraint.c)
            {
                scroll_offset.y = -(offs.y - constraint.c);
                scroll_panel->set_scroll_position(scroll_offset);
            }
            else
            {
                scroll_panel->set_scroll_position(scroll_offset);
            }
        }
    }
}


void SelectionWidget::set_selection(int set)
{
    if (set > selection_names.size() - 1)
    {
        set = selection_names.size() - 1;
    }
    else if (set < 0)
    {
        set = 0;
    }

    selection = set;
}


vector4d SelectionWidget::get_child_widget_padding() const
{
    if (child_widget)
    {
        return child_widget->get_padding();
    }

    return vector4d();
}


vector2d SelectionWidget::get_child_widget_size() const
{
    if (child_widget)
    {
        return child_widget->get_size();
    }

    return vector2d();
}


void SelectionWidget::update_child_size(bool b_recursive)
{
    if (child_widget)
    {
        child_widget->update_size(b_recursive);
    }
}


void SelectionWidget::on_focus_received()
{
    // only update size and rebuild if term size has changed
    if (term_size_cache != vector2d(term_w(), term_h()))
    {
        update_size();
        rebuild(true);
    }

    show();

    WIDGET_MAN.draw_widgets(this, false, false);
}


void SelectionWidget::on_focus_lost()
{
    delete_self();
}


void SelectionWidget::receive_left_click(vector2d coord, TermWidget* clicked, TermWidget* source)
{
    // select item
    if (clicked && vbox)
    {
        for (int i = 0; i < vbox->children.size(); ++i)
        {
            std::shared_ptr<TermWidget> c = vbox->children[i];
            if (c.get() == clicked)
            {
                TextWidget* item = dynamic_cast<TextWidget*>(clicked);
                if (item)
                {
                    selection = i;
                    rebuild(true);
                    WIDGET_MAN.draw_widgets(
                        this,
                        false,
                        false);
                    activate_selection();

                    return;
                }
            }
        }
    }

    // bubble up
    if (parent_widget)
    {
        parent_widget->receive_left_click(coord, this);
    }
}

