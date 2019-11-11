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

struct data_4chan;
class Post4chanWidget;
class ColorBlockWidget;
class ScrollPanelWidget;
class BoxWidget;
class VerticalBoxWidget;
class TextWidget;


class Thread4chanWidget : public ChanWidget
{

public:

    Thread4chanWidget(data_4chan& chan_data, bool b_update = true);


protected:

    std::string thread_url;
    std::string board;
    std::string thread_num_str;
    std::string thread_subject;
    std::shared_ptr<VerticalBoxWidget> main_vbox;
    std::shared_ptr<BoxWidget> header;
    std::shared_ptr<TextWidget> header_info;
    std::vector<term_word> base_header_text;
    std::shared_ptr<BoxWidget> footer;
    std::shared_ptr<TextWidget> footer_info;
    std::shared_ptr<BoxWidget> posts_box;
    std::shared_ptr<VerticalBoxWidget> posts_vbox;
    std::vector<term_word> base_footer_text;
    std::shared_ptr<ScrollPanelWidget> scroll_panel;

    bool b_archived;
    bool b_auto_update;
    bool b_manual_update;
    std::map<int, std::shared_ptr<Post4chanWidget>> post_map;

    // seconds between auto refreshes
    int auto_ref_s;
    uint32_t time_cache;
    std::chrono::milliseconds auto_refresh_interval;
    std::chrono::milliseconds auto_refresh_counter;
    std::chrono::milliseconds reload_flash_interval;
    // flip flop symbol
    int reload_flash_num;
    int reload_flash_count;
    bool b_reload_flash_sym;
    virtual void tick_event(std::chrono::milliseconds delta) override;

    bool b_reloading;
    bool b_can_save;

    Post4chanWidget* selected_post;
    std::wstring footer_countdown;

    void update_header_info();
    void save_to_disk() const;
    void delete_save_file() const;


public:

    std::shared_ptr<VerticalBoxWidget> get_posts_vbox() { return posts_vbox; };
    std::shared_ptr<ScrollPanelWidget> get_scroll_panel() { return scroll_panel; };

    bool is_saved() const;

    virtual bool on_received_update(data_4chan& chan_data) override;

    void scroll_to_post(int post_num);
    void select_post(Post4chanWidget* post);

    // updates all widget sizes and redraws,
    // but does not do a full rebuild from the json file
    virtual void refresh(bool b_draw_term = true) override;

    std::shared_ptr<Post4chanWidget> get_post(int post_num);

    std::string get_board() const { return board; };
    std::string get_thread_num_str() const { return thread_num_str; }; 
    void get_thread_key() const { return thread_url; };
    void add_reply(int post, int reply);
    virtual bool receive_img_packet(img_packet& pac) override;

    virtual bool handle_key_input(const tb_event& input_event, bool b_bubble_up = true) override;
    virtual void handle_term_resize_event() override;
    virtual void on_focus_received();

    virtual void rebuild(bool b_rebuild_children = true) override;
    virtual void child_widget_size_change_event() override;

    virtual void draw_children(vector4d constraint = vector4d(-1)) const override;

    virtual vector2d get_child_widget_size() const override;
    virtual void update_child_size(bool b_recursive = false) override;
};

