/**
 * Comfy
 *
 *  Copyright 2019 by Wolfish <wolfish@airmail.cc>
 *  https://wolfish.neocities.org/soft/comfy/
 *
 *  Licensed under the GPL v2.0 only.
 */
#include "fileops.h"
#include <experimental/filesystem>
#include <fstream>

using namespace experimental::filesystem;


bool FileOps::valid_dir(string dir)
{
    if (dir.empty()) return false;

    // ensure that the directory path is inside of comfy's data directory
    if (dir.substr(0, DATA_DIR.length()).find(DATA_DIR, 0) != string::npos)
    {
        return true;
    }

    return false;
}


bool FileOps::file_exists(const string& file_path)
{
    if (file_path.empty()) return false;

    if (FILE *file = fopen(file_path.c_str(), "r")) {
        fclose(file);
        return true;
    } else {
        return false;
    }   
}


void FileOps::mkdir(const string& path)
{
    if (!valid_dir(path)) return;

    create_directories(path);
}


void FileOps::write_file(const string& file_path, const string& file_name, const char* buf, int buf_size)
{
    if (!valid_dir(file_path) || file_name.empty()) return;

    mkdir(file_path);
    ofstream outfile (file_path + file_name, ofstream::binary);
    outfile.write(buf, buf_size);
    outfile.close();
}


vector<string> FileOps::get_dir_contents(string path)
{
    vector<string> dir;
    if (path.empty()) return dir;

    for (const auto & entry : directory_iterator(path))
        dir.emplace_back(entry.path());

    return dir;
}


void FileOps::delete_all_in_dir(string path)
{
    if (!valid_dir(path)) return;

    if (is_directory(path))
    {
        vector<string> ls = get_dir_contents(path);
        for (auto& f : ls)
        {
            remove_all(f);
        }
    }
}


void FileOps::delete_file(string path)
{
    if (!valid_dir(path)) return;

    if (is_regular_file(path))
    {
        remove(path);
    }
}


bool FileOps::dir_contains_save_file(string dir)
{
    vector<string> ls = get_dir_contents(dir);
    for (auto& f : ls)
    {
        if (f.compare(SAVE_FILE) == 0)
        {
            return true;
        }
    }

    return false;
}


vector<string> FileOps::get_lines_in_file(string file_path)
{
    if (!exists(file_path) || !is_regular_file(file_path))
    {
        return vector<string>();
    }

    ifstream f(file_path);
    stringstream buffer;
    buffer << f.rdbuf();

    return split(buffer.str(), "\n");
}


void FileOps::read_file(stringstream& out, string path)
{
    ifstream f(path);
    if (f)
    {
        out << f.rdbuf();
        f.close();
    }
    //string contents((istreambuf_iterator<char>(f)), 
    //    istreambuf_iterator<char>());
}


static vector<string> FileOps::get_all_dirs_containing_file_name(string start_dir, string file_name)
{
    if (!valid_dir(start_dir)) return vector<string>();

    vector<string> dirs;
    if (exists(start_dir + "/" + file_name)) dirs.push_back(start_dir);

    vector<string> ls = get_dir_contents(start_dir);
    for (auto& f : ls)
    {
        if (is_directory(f))
        {
            vector<string> ds = get_all_dirs_containing_file_name(f, file_name);
            for (auto& d : ds)
            {
                dirs.emplace_back(d);
            }
        }
    }

    return dirs;
}


bool FileOps::recursively_delete_all_unsaved_dirs(string dir)
{
    if (!valid_dir(dir) || !is_directory(dir)) return false;

    bool b_contains_folder = false;
    bool b_contains_save_file = false;
    vector<string> ls = get_dir_contents(dir);
    for (auto& f : ls)
    {
        // if f not deleted and is a dir...
        if (!recursively_delete_all_unsaved_dirs(f) &&
            is_directory(f))
        {
            b_contains_folder = true;
        }
        // if f is a comfy save file
        else if (is_regular_file(f) && f.compare(dir + "/" + SAVE_FILE) == 0)
        {
            b_contains_save_file = true;
        }
    }

    // delete dir if contains no folders and does not contain a save file
    // e.g. a directory full of images and no save file would be deleted
    if (dir.compare(DATA_DIR) != 0)// don't delete data directory
    {
        // remove entire directory
        if (!b_contains_folder && !b_contains_save_file)
        {
            remove_all(dir);
            return true;
        }
        // remove only files in directory, keeping folders
        else if (b_contains_folder && !b_contains_save_file)
        {
            for (auto& f : ls)
            {
                if (!is_directory(f) && is_regular_file(f))
                {
                    remove(f);
                }
            }
        }
    }

    return false;
}













