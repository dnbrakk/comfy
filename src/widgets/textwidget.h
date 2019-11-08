/**
 * Comfy
 *
 *  Copyright 2019 by Wolfish <wolfish@airmail.cc>
 *  https://wolfish.neocities.org/soft/comfy/
 *
 *  Licensed under the GPL v2.0 only.
 */
#pragma once
#include "termwidget.h"


class TextWidget : public TermWidget
{

public:

    TextWidget(vector2d _offset = vector2d(), vector4d _padding = vector4d(), uint32_t _bg_color = 16, uint32_t _fg_color = 255);


protected:

    std::vector<term_word> term_words;
    bool b_parse_4chan;
    int widest_row;
    int cell_index;
    // used by append_char()
    int consecutive_space_count;
    std::map<int, term_word> format_override;

    // returns true if max height has been reached
    bool push_row(std::vector<tb_cell>& row);
    void make_cells(std::vector<tb_cell>& row, term_word& word);


public:

    std::vector<term_word>& get_term_words() { return term_words; };
    int get_cell_index() const { return cell_index; };
    void set_parse_4chan(bool b_parse) { b_parse_4chan = b_parse; };

    // override the cell formatting at index
    void override_format(
        int index, 
        uint32_t bg,
        uint32_t fg,
        bool b_bold = false,
        bool b_underline = false,
        bool b_reverse = false
    );

    void clear_override_formatting() { format_override.clear(); };

    static void sanitize_text(std::wstring& str);
    static void parse_4chan(std::vector<term_word>& words);

    virtual void rebuild(bool b_rebuild_children = true) override;

    // returns the integer portion of s
    // if s is a post num quote (e.g. ">>3264217777")
    // returns -1 if s is not a post num quote
    static int parse_post_num_quote(const std::wstring& s);

    void append_text(const std::vector<term_word>& words);

    // individual chars

    // index of -1 appends to end of last word
    // if appending to an existing word, the color and style
    // is the same as that word
    void append_char(
        wchar_t ch,
        bool b_rebuild = false,
        int index = -1,
        bool b_bold = false,
        bool b_underline = false,
        bool b_reverse = false,
        uint32_t bg = -1,
        uint32_t fg = -1
    );

    void remove_char(int index, bool b_rebuild = false);

    // sanitized and parsed text

    void append_text(
        const std::wstring& str,
        bool b_rebuild = false,
        bool b_bold = false,
        bool b_underline = false,
        bool b_reverse = false,
        uint32_t bg = -1,
        uint32_t fg = -1
    );

    void append_text(
        const char* str,
        bool b_rebuild = false,
        bool b_bold = false,
        bool b_underline = false,
        bool b_reverse = false,
        uint32_t bg = -1,
        uint32_t fg = -1
    );

    void append_text(
        const std::string& str,
        bool b_rebuild = false,
        bool b_bold = false,
        bool b_underline = false,
        bool b_reverse = false,
        uint32_t bg = -1,
        uint32_t fg = -1
    );

    // raw text

    void append_raw_text(
        const std::wstring& str,
        bool b_rebuild = false,
        bool b_bold = false,
        bool b_underline = false,
        bool b_reverse = false,
        uint32_t bg = -1,
        uint32_t fg = -1
    );

    void append_raw_text(
        const char* str,
        bool b_rebuild = false,
        bool b_bold = false,
        bool b_underline = false,
        bool b_reverse = false,
        uint32_t bg = -1,
        uint32_t fg = -1
    );

    void append_raw_text(
        const std::string& str,
        bool b_rebuild = false,
        bool b_bold = false,
        bool b_underline = false,
        bool b_reverse = false,
        uint32_t bg = -1,
        uint32_t fg = -1
    );

    std::wstring get_word_at_coord(vector2d coord) const;
    // index in text string at term coord
    // if y > num rows, y = num rows
    // if x > width of row, x = width of row
    // returns -1 if given invalid coords
    int get_index_at_coord(vector2d coord) const;
    // coord in term of index in text string
    vector2d get_coord_of_index(int index) const;

    void clear_text() { term_words.clear(); widest_row = 0; };

};

