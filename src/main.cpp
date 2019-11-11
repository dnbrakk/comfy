/**
 * Comfy
 *
 *  Copyright 2019 by Wolfish <wolfish@airmail.cc>
 *  https://wolfish.neocities.org/soft/comfy/
 *
 *  Licensed under the GPL v2.0 only.
 */
#include "comfy.h"
#include <cstdio>
#include "fileops.h"
#include "netops.h"
#include "widgetman.h"
#include "imgman.h"
#include "../getoptpp/getopt_pp.h"

using namespace std;

int MAX_THREADS = -1;

// ------ defined extern in comfy.h:
std::string DATA_DIR = ".comfy/";
std::string IMAGEBOARDS_DIR = DATA_DIR + "imageboards/";
std::string TEMP_DIR = DATA_DIR + "tmp/";
std::string FLAGS_DIR = DATA_DIR + "4chan/flags/";
Colors::color_scheme COLO = Colors::COMFYBLUE;
bool DISPLAY_IMAGES = true;
// ------


void init_files()
{
    string home_dir = getenv("HOME");
    DATA_DIR = home_dir + "/.comfy/";
    IMAGEBOARDS_DIR = DATA_DIR + "imageboards/";
    TEMP_DIR = DATA_DIR + "tmp/";
    FLAGS_DIR = DATA_DIR + "imageboards/4chan/flags/";

    FileOps::mkdir(DATA_DIR);

    // redirect stderr to log file (truncates)
    std::string err_file = DATA_DIR;
    err_file += "error_log.txt";
    freopen(err_file.c_str(), "w", stderr);
}


void clean_up_files()
{
    FileOps::recursively_delete_all_unsaved_dirs(IMAGEBOARDS_DIR);
    FileOps::recursively_delete_all_unsaved_dirs(TEMP_DIR);
}


void parse_opts(int argc, char* argv[])
{
    // parse command line options
    GetOpt::GetOpt_pp ops(argc, argv);

    if (ops >> GetOpt::OptionPresent('h', "help"))
    {
        string help =   "Arguments:\n";
        help +=         "    -d    or  --disable-images       Disable images\n";
        help +=         "    -m n  or  --max-threads n        Set max number of concurrent threads, where n is max number\n";
        help +=         "    -v    or  --version              Print version and exit\n";
        help +=         "    -h    or  --help                 Print help (this message) and exit\n";
        help +=         "\n";
        help +=         "Controls:\n";
        help +=         "    Backspace/CTRL+H                 Go to homescreen, or back one level in list\n";
        help +=         "    CTRL+Q                           Quit\n";
        help +=         "    CTRL+X                           Close catalog or thread\n";
        help +=         "    CTRL+R                           Reload the focused page from the network\n";
        help +=         "    CTRL+A                           Enable/disable auto-reload\n";
        help +=         "    CTRL+S                           Save currently focused thread\n";
        help +=         "    F5                               Do a hard refresh of the screen\n";
        help +=         "    Tab                              Switch between currently opened pages\n";
        help +=         "    Space/Enter                      Choose selection in list\n";
        help +=         "    Up/Down Arrow Keys               Scroll up/down one line at a time\n";
        help +=         "    Left/Right Arrow Keys            Scroll up/down one screen at a time\n";
        help +=         "    Page Up/Down                     Scroll up/down one screen at a time\n";
        help +=         "    Home                             Go to top of page\n";
        help +=         "    End                              Go to bottom of page\n";
        help +=         "    Mouse Wheel                      Scroll the currently focused page\n";
        help +=         "    Left-Click                       Open a thread in a catalog,\n";
        help +=         "                                     full screen/close an image in a thread,\n";
        help +=         "                                     or go to a post in a thread (by clicking a post number link)\n";
        std::cout << help;
        exit(0);
    }

    if (ops >> GetOpt::OptionPresent('v', "version"))
    {
        std::string ver = "Comfy ";
        ver += VERSION;
        std::cout << ver << std::endl;
        exit(0);
    }

    // disable images
    DISPLAY_IMAGES = !(ops >> GetOpt::OptionPresent('d', "disable-images"));

    // maximum concurrent threads
    if (ops >> GetOpt::OptionPresent('m', "max-threads"))
    {
        if (ops >> GetOpt::Option('m', "max-threads", MAX_THREADS));
    }
}


void load_urls(int argc, char* argv[])
{
    // parse command line options
    GetOpt::GetOpt_pp ops(argc, argv);

    // get args that were potentially URLs
    std::vector<std::string> args;
    ops >> GetOpt::GlobalOption(args);
    for (auto& s : args)
    {
        url_parser parser(s);
        if (parser.pagetype == pt_board_catalog ||
            parser.pagetype == pt_thread)
        {
            NetOps::http_get__4chan_json(s, "", true, 0);
        }
    }
}


int main(int argc, char* argv[])
{
    // init
    init_files();
    parse_opts(argc, argv);
    if (DISPLAY_IMAGES) IMG_MAN.init();
    NetOps::init();
    THREAD_MAN.init(MAX_THREADS);

    // load urls from args
    load_urls(argc, argv);

    // run
    WIDGET_MAN.run();

    // shutdown
    THREAD_MAN.shutdown();
    NetOps::shutdown();
    if (DISPLAY_IMAGES)IMG_MAN.shutdown();
    clean_up_files();

    return 0;
}

