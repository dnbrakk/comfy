/**
 * Comfy
 *
 *  Copyright 2019 by Wolfish <wolfish@airmail.cc>
 *  https://wolfish.neocities.org/soft/comfy/
 *
 *  Licensed under the GPL v2.0 only.
 */
#pragma once
#include "thread4chanwidget.h"

struct data_4chan;
class CatalogThread4chanWidget;
class ColorBlockWidget;
class ScrollPanelWidget;
class BoxWidget;
class TextWidget;


class Catalog4chanWidget : public Thread4chanWidget
{

public:

    Catalog4chanWidget(data_4chan& chan_data, bool b_update = true);


protected:

    std::string board_url;
    std::map<int, std::shared_ptr<CatalogThread4chanWidget>> thread_map;
    CatalogThread4chanWidget* selected_thread;
    vector2d thread_box_size;


public:

    virtual bool on_received_update(data_4chan& chan_data) override;

    void select_thread(CatalogThread4chanWidget* thread);
    std::shared_ptr<CatalogThread4chanWidget> get_thread(int post_num);

    virtual void rebuild(bool b_rebuild_children = true) override;
    virtual bool receive_img_packet(img_packet& pac) override;

    //virtual bool handle_key_input(const tb_event& input_event) override;

    virtual void receive_left_click(vector2d coord, TermWidget* clicked = nullptr, TermWidget* source = nullptr) override;
};

