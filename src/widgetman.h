/**
 * Comfy
 *
 *  Copyright 2019 by Wolfish <wolfish@airmail.cc>
 *  https://wolfish.neocities.org/soft/comfy/
 *
 *  Licensed under the GPL v2.0 only.
 */
#pragma once
#include "comfy.h"

class TermWidget;
class TextWidget;
class ScrollPanelWidget;
class SelectionWidget;
class HomescreenWidget;
struct data_4chan;


class WidgetMan
{

public:

    // leave the constructor empty so that 
    // nothing is executed when the static
    // instance is fetched
    WidgetMan() {};

    // delete these functions for use as singleton
    WidgetMan(WidgetMan const&)         = delete;
    void operator=(WidgetMan const&)    = delete;

    static WidgetMan& get_instance()
    {
        static WidgetMan widgetman;
        return widgetman;
    }

    void run();
    void stop();


protected:

    void init();
    void shutdown();

    bool b_init;

    // if set to false, run() exits
    bool b_run;

    std::vector<std::shared_ptr<TermWidget>> widget_stack;
    std::shared_ptr<TermWidget> focused_widget;
    // if draw_controller is not nullptr,
    // only the draw_controller widget can make calls
    // to draw_widgets(), otherwise any widget can call draw_widgets().
    // this is useful sometimes, as e.g. thread posts
    // may refresh the screen when receiving images from
    // worker threads in the background, which could
    // unintentionally wipe the screen while another widget
    // is focused and being drawn
    TermWidget* draw_controller;

    std::shared_ptr<HomescreenWidget> homescreen;

    void load_image_data();
    void load_chan_data();
    void load_4chan_data(data_4chan& chan_data);

    static void switch_widget(std::shared_ptr<TermWidget> wgt);

    vector2d term_size_cache;

    void tick_widgets();

    std::chrono::microseconds last_tick_time;

    // cell coordinates in this vector are drawn
    // to screen by termbox when draw_widgets() is called.
    // can be used to, for example, prevent drawing cells
    // where an image is to be rendered this frame.
    std::vector<vector2d> null_cells;

    // sets null cells in termbox buffer and then empties
    // null_cells vector if b_clear is true
    void apply_null_cells(bool b_clear = true);

    // cells to be drawn using termbox in order from
    // 0 to end of vector, comprising one draw frame
    std::vector<term_cell> termbox_frame;
    std::vector<term_cell> blank_termbox_frame;

    // coordinates in this vector get force cleared in draw()
    std::vector<vector2d> artifact_remove;

    // if false, IMG_MAN redraw_buffer() is not called in tick
    // and draw_widgets()
    bool b_draw_img_buffer;


public:

    void set_draw_img_buffer(bool b_set) { b_draw_img_buffer = b_set; };

    void switch_to_homescreen();
    void open_switch_widget();

    std::shared_ptr<SelectionWidget> switch_widget_select;

    // if b_steal_focus is true, when the data packed is received
    // from ThreadManager, the widget that receives the packet
    // will be focused
    static void open_thread(std::string url, bool b_steal_focus = false);

    vector2d get_term_size() { return term_size_cache; };

    void handle_key_input(const tb_event& input_event);
    // sets newly added widget as selected.
    // returns the added widget, nullptr if widget not added.
    // widgets cannot be added if widget->get_id() == ""
    std::shared_ptr<TermWidget> add_widget(std::shared_ptr<TermWidget> widget, bool b_focus = true);
    // returns true if widget was removed
    bool remove_widget(std::shared_ptr<TermWidget> widget);
    bool remove_widget(TermWidget* widget);
    void focus_widget(std::shared_ptr<TermWidget> widget);

    void clear_termbox_frame(bool b_resize = false);
    void add_cell_to_frame(term_cell& cell);
    // if b_clear is true the termbox_frame vector is cleared
    void put_termbox_frame(bool b_clear = true);
    void add_null_cell(vector2d cell);
    void remove_image_artifact(vector2d coord);
    // if no widget is supplied for draw_widget,
    // then focused_widget is drawn by default
    void draw_widgets(TermWidget* draw_widget = nullptr, bool b_clear_cells = true, bool b_clear_images = true);
    void termbox_draw();
    void termbox_clear(uint32_t color = 0);
    // wipe all image artifacts and term cells
    void hard_refresh();

    void handle_term_resize_event(const tb_event& resize_event);

    std::shared_ptr<TermWidget> get_widget(std::string key);

    void move_to_front(std::shared_ptr<TermWidget> widget, bool b_focus = true);

};

