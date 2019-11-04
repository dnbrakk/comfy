/**
 * Comfy
 *
 *  Copyright 2019 by Wolfish <wolfish@airmail.cc>
 *  https://wolfish.neocities.org/soft/comfy/
 *
 *  Licensed under the GPL v2.0 only.
 */
#pragma once
#include "../comfy.h"

struct http_image_req;
struct img_packet;


class TermWidget
{

public:

    TermWidget(vector2d _offset = vector2d(), vector4d _padding = vector4d(), vector4d _child_padding = vector4d(), uint32_t _bg_color = TB_BLACK, uint32_t _fg_color = TB_WHITE,  bool _b_fullscreen = false);


protected:

    // if id == "" then it cannot be added to WidgetMan to be drawn
    std::string id;
    // displayed in switch widget selection dialog etc.
    std::string title;

    TermWidget* parent_widget;
    std::shared_ptr<TermWidget> child_widget;

    vector2d offset;
    // received from other widgets
    vector2d inherited_offset;

    // padding relative to parent_widget.
    // may be modified by other widgets,
    // but will always have default_padding
    // and inherited_padding added to it.
    //
    // padding.a = left
    // padding.b = top
    // padding.c = right
    // padding.d = bottom
    //
    // the padding the widget was initialized with
    vector4d default_padding;
    // padding dictated by the parent widget
    vector4d inherited_padding;
    // padding that gets added to child widgets
    vector4d child_padding;

    // horizontal and vertical sizing
    e_widget_sizing h_sizing;
    e_widget_sizing v_sizing;

    // horizontal and vertical alignment
    e_widget_align h_align;
    e_widget_align v_align;

    // 2D array of termbox cells
    std::vector<std::vector<tb_cell>> cells;

    // if true, widget always fills entire terminal
    bool b_fullscreen;
    vector2d size;
    vector2d min_size;

    uint32_t bg_color;
    uint32_t fg_color;

    // if true, widget will not be drawn
    bool b_hidden;

    // TODO: implement this
    // if true, widget has a size of zero, and will not be drawn
    // if a widget has size zero, parent widgets won't factor in its
    // size when calculating their own size (e.g. boxes)
    //bool b_collapsed;

    vector2d term_size_cache;
    vector4d last_constraint;

    // whether or not this widget receives tick updates
    bool b_ticks;

    std::chrono::milliseconds tick_rate;
    std::chrono::milliseconds last_tick_time;

    // whether or not the Widget Manager should redraw
    // the image buffer while this widget is focused
    bool b_draw_img_buffer;

    // optionally blast out image artifacts
    // where widget will be drawn
    bool b_remove_img_artifacts_on_draw;

    // if true, when widget is focused only this widget
    // is allowed to call draw_widgets() in Widget Manager,
    // preventing other widgets from drawing to term
    bool b_take_draw_control;

    // whether or not widget should appear in switch widget selection list
    bool b_can_switch_to;


public:

    virtual void set_child_widget(std::shared_ptr<TermWidget> _child_widget, bool b_rebuild = true);
    std::shared_ptr<TermWidget> get_child_widget() { return child_widget; };

    void set_id(std::string _id) { id = _id; };
    std::string get_id() const { return id; };
    virtual std::string get_title() const;

    // returns true if this widget is child or sub-child of wgt,
    // or if this widget == wgt
    bool is_child_of(TermWidget* wgt);

    void set_bg_color(uint32_t _bg_color) { bg_color = _bg_color; };
    void set_fg_color(uint32_t _fg_color) { fg_color = _fg_color; };
    uint32_t get_bg_color() const { return bg_color; };
    uint32_t get_fg_color() const { return fg_color; };

    std::vector<std::vector<tb_cell>>& get_cells() { return cells; };

    bool can_switch_to() const { return b_can_switch_to; };
    void set_can_switch_to(bool b_set) { b_can_switch_to = b_set; };

    bool should_draw_img_buffer() const { return b_draw_img_buffer; };
    void set_draw_img_buffer(bool b_set) { b_draw_img_buffer = b_set; };

    void set_remove_img_artifacts_on_draw(bool b_set) {
        b_remove_img_artifacts_on_draw = b_set; };

    bool should_take_draw_control() const { return b_take_draw_control; };
    void set_take_draw_control(bool b_set) { b_take_draw_control = b_set; };

    int term_h();
    int term_w();

    bool ticks() const { return b_ticks; };
    void tick();
    virtual void tick_event(std::chrono::milliseconds delta) {};
    void set_tick_rate(float rate);

    void set_h_sizing(e_widget_sizing ws) { h_sizing = ws; };
    void set_v_sizing(e_widget_sizing ws) { v_sizing = ws; };
    e_widget_sizing get_h_sizing() const { return h_sizing; };
    e_widget_sizing get_v_sizing() const { return v_sizing; };

    void set_h_align(e_widget_align wa) { h_align = wa; };
    void set_v_align(e_widget_align wa) { v_align = wa; };
    e_widget_align get_h_align() const { return h_align; };
    e_widget_align get_v_align() const { return v_align; };

    void set_parent_widget(TermWidget* _parent_widget);
    TermWidget* get_parent_widget() { return parent_widget; };

    virtual bool handle_key_input(const tb_event& input_event, bool b_bubble_up = true);
    // returns the child widget (searched recursively)
    // at coordinates, or self. used for determining
    // what widget was clicked
    virtual TermWidget* get_topmost_child_at(vector2d coord);

    // clicked is the widget that was clicked
    // source is widget that the click was bubbled from (e.g. the child)
    virtual void receive_left_click(vector2d coord, TermWidget* clicked = nullptr, TermWidget* source = nullptr);
    virtual void receive_right_click(TermWidget* clicked = nullptr, TermWidget* source = nullptr);
    virtual void receive_middle_click(TermWidget* clicked = nullptr, TermWidget* source = nullptr);

    virtual void on_focus_received();
    virtual void on_focus_lost();

    bool is_fullscreen() const { return b_fullscreen; };

    void update_size(bool b_recursive = false);
    void set_size(int w, int h);
    void set_size(vector2d _size);
    void set_min_width(int w) { min_size.x = w; };
    void set_min_height(int h) { min_size.y = h; };
    virtual void size_change_event() {};
    vector2d get_size() const { return size; };

    int get_width_constraint() const;
    int get_height_constraint() const;
    int get_max_parent_width() const;
    int get_max_parent_height() const;

    vector4d get_child_padding() const { return child_padding; };
    void set_child_padding(vector4d _child_padding) { child_padding = _child_padding; };
    void add_child_padding(vector4d add) { child_padding += add; };

    virtual vector2d get_child_widget_size() const;
    virtual vector4d get_child_widget_padding() const;
    virtual void update_child_size(bool b_recursive = false) {};

    void set_offset(vector2d _offset) { offset = _offset; };
    vector2d get_offset() const { return offset; };
    void add_offset(vector2d add) { offset += add; };

    void set_inherited_offset(vector2d _offset) { inherited_offset = _offset; };
    vector2d get_inherited_offset() const { return inherited_offset; };
    void add_inherited_offset(vector2d add) { inherited_offset += add; };

    // returns absolute position relative to terminal
    vector2d get_absolute_offset() const;
    vector2d get_alignment_offset() const;
    vector4d get_padding() const;
    void add_inherited_padding(vector4d pad);
    void set_inherited_padding(vector4d pad);
    vector4d get_inherited_padding() const { return inherited_padding; };
    void clear_inherited_padding();

    // can be used as a lighter version of rebuild
    virtual void refresh(bool b_draw_term = true);
    virtual void rebuild(bool b_rebuild_children = true) {};
    virtual void handle_term_resize_event();
    virtual void child_widget_size_change_event() {};

    virtual bool receive_img_packet(img_packet& pac) {};

    virtual void draw(vector4d constraint = vector4d(-1), bool b_draw_children = true) const;
    virtual void draw_children(vector4d constraint = vector4d(-1)) const;

    // returns the width and height this widget
    // currently takes up inside the terminal screen
    // (i.e. it omits cells that are off-screen)
    virtual vector2d size_on_screen() const;
    
    void set_hidden(bool _b_hidden) { b_hidden = _b_hidden; };
    bool hidden() const { return b_hidden; };
    void hide() { set_hidden(true); };
    void show() { set_hidden(false); };

    virtual void delete_self();

    //void set_collapsed(bool b_set) { b_collapsed = b_set; };
    //void collapse() { set_collapsed(true); };
    //void uncollapse() { set_collapsed(false); };
    //bool is_collapsed() const { return b_collapsed; };
};

