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

using namespace std;


class FileOps
{
public:

    static bool valid_dir(string dir);
    static bool file_exists(const string& file_path);
    static void mkdir(const string& path);
    static void write_file(const string& file_path, const string& file_name, const char* buf, int buf_size);
    static vector<string> get_dir_contents(string path);
    static void delete_all_in_dir(string path);
    static void delete_file(string path);
    static bool dir_contains_save_file(string dir);
    // returns true if no parse errors
    static void read_file(stringstream& out, string path);
    // returns a vector of string, one string per line in the file
    static vector<string> get_lines_in_file(string file_path);
    // recursively searches dirs beginning with start_dir
    // for files named file_name, returning a vector of dirs
    // that contain that file
    static vector<string> get_all_dirs_containing_file_name(string start_dir, string file_name);
    // returns true if dir itself is deleted
    static bool recursively_delete_all_unsaved_dirs(string dir);
};

