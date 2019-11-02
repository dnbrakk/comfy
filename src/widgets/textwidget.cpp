/**
 * Comfy
 *
 *  Copyright 2019 by Wolfish <wolfish@airmail.cc>
 *  https://wolfish.neocities.org/soft/comfy/
 *
 *  Licensed under the GPL v2.0 only.
 */
#include "textwidget.h"
#include "../widgetman.h"


TextWidget::TextWidget(vector2d _offset, vector4d _padding, uint32_t _bg_color, uint32_t _fg_color)
: TermWidget(_offset, _padding, vector4d(), _bg_color, _fg_color)
{
    set_h_align(e_widget_align::wa_left);
    set_v_align(e_widget_align::wa_top);
    set_h_sizing(e_widget_sizing::ws_fill);
    set_v_sizing(e_widget_sizing::ws_dynamic);
    b_parse_4chan = false;
    widest_row = 0;
    cell_index = 0;
}


void TextWidget::override_format(int index, uint32_t bg, uint32_t fg, bool b_bold, bool b_underline, bool b_reverse)
{
    format_override[index] =
        term_word(L"", bg, fg, b_bold, b_underline, b_reverse);
}


// returns true if max height has been reached
bool TextWidget::push_row(std::vector<tb_cell>& row)
{
    // do not exceed maximum height, if v sizing is not dynamic
    if (get_v_sizing() == ws_fixed ||
        get_v_sizing() == ws_fill ||
        get_v_sizing() == ws_fill_managed)
    {
        int max_height = get_height_constraint();
        if (cells.size() + 1 > max_height)
        {
            return true;
        }
    }

    cells.push_back(row);
    if (row.size() > widest_row)
    {
        widest_row = row.size();
    }
    row.clear();

    return false;
}


void TextWidget::make_cells(std::vector<tb_cell>& row, term_word& word)
{
    if (word.b_newline)
    {
        push_row(row);
    }
    else
    {
        for (auto& ch : word.text)
        {
            tb_cell cell;
            cell.ch = ch;
            bool b_bold = false;
            bool b_underline = false;
            bool b_reverse = false;
            
            try
            {
                term_word& fw = format_override.at(cell_index);
                cell.bg = fw.bg;
                cell.fg = fw.fg;
                b_bold = fw.b_bold;
                b_underline = fw.b_underline;
                b_reverse = fw.b_reverse;
            }
            catch (const std::out_of_range& oor)
            {
                cell.bg = word.bg;
                cell.fg = word.fg;
                b_bold = word.b_bold;
                b_underline = word.b_underline;
                b_reverse = word.b_reverse;
            }

            if (b_bold)
            {
                cell.fg = cell.fg | TB_BOLD;
            }

            if (b_underline)
            {
                cell.fg = cell.fg | TB_UNDERLINE;
            }

            if (b_reverse)
            {
                cell.fg = cell.fg | TB_REVERSE;
            }

            row.push_back(cell);
            cell_index++;
        }
    }
}


void TextWidget::rebuild(bool b_rebuild_children)
{
    cells.clear();
    widest_row = 0;
    cell_index = 0;

    // h_sizing of auto: no width limit
    int max_width = std::numeric_limits<int>::max() - 1;
    if (get_h_sizing() == ws_fixed)
    {
        max_width = size.x;
    }
    else if (get_h_sizing() == ws_fullscreen)
    {
        max_width = term_w();
    }
    else if (get_h_sizing() == ws_fill || get_h_sizing() == ws_dynamic)
    {
        // set to fill so that size of parent is used for max width
        set_h_sizing(e_widget_sizing::ws_fill);
        update_size();
        max_width = size.x;
    }

    std::vector<tb_cell> row;

    for (auto& word : term_words)
    {
        int space = 0;
        if (row.size() > 0) space = 1;
        // start new row if:
        //      row is not empty and word is not longer than max row width
        //      and
        //      word + row would exceed max row width
        if (!(space == 0 && word.text.length() > max_width) &&
            row.size() + space + word.text.length() > max_width)
        {
            if (push_row(row)) goto end;
        }
        // space
        else if (space > 0)
        {
            term_word w_sp(L" ", word.bg, word.fg, false, false, false);
            make_cells(row, w_sp);
        }

        // word is longer than max row width
        if (word.text.length() > max_width)
        {
            std::wstring spam = word.text;
            while(!spam.empty())
            {
                if (spam.length() > max_width)
                {
                    std::wstring chop = spam.substr(0, max_width);
                    spam = spam.substr(max_width);
                    term_word w = word;
                    w.text = chop;
                    make_cells(row, w);

                    if (push_row(row)) goto end;
                }
                else
                {
                    term_word w = word;
                    w.text = spam;
                    make_cells(row, w);
                    spam = std::wstring();
                }
            }
        }
        else
        {
            make_cells(row, word);
        }
    }

    if (row.size() > 0)
    {
        push_row(row);
    }

    end:

    if (get_h_sizing() == ws_fixed)
    {
        set_size(size.x, cells.size());
    }
    else
    {
        set_size(widest_row, cells.size());
    }

    // set to dynamic so that the size set here is not altered
    // by calls to update_size()
    if (get_h_sizing() == ws_fill)
    {
        set_h_sizing(ws_dynamic);
    }
}


// returns the integer portion of s
// if s is a post num quote (e.g. ">>3264217777")
// returns -1 if s is not a post num quote
int TextWidget::parse_post_num_quote(const std::wstring& s)
{
    for (int i = 0; i < s.length(); ++i)
    {
        auto ch = s[i];
        if (i + 1 < s.length() && ch == '>' && s[i + 1] == '>')
        {
            std::string snum;
            for (int j = i + 2; j < s.length(); ++j)
            {
                if (std::isdigit(s[j]))
                {
                    snum += s[j];
                }
                else
                {
                    return -1;
                }

                if (j + 1 == s.length())
                {
                    try
                    {
                        int num = std::stoi(snum);
                        return num;
                    }
                    catch (...)
                    {
                        return -1;
                    }
                }
            }
        }
    }

    return -1;
}


void TextWidget::parse_4chan(std::vector<term_word>& words)
{
    bool b_prev_was_newline = true;
    bool b_greentext = false;
    for (auto& w : words)
    {
        if (w.b_newline)
        {
            b_prev_was_newline = true;
            b_greentext = false;
            continue;
        }

        std::wstring& text = w.text;

        // begin greentext
        if (b_prev_was_newline && text.length() > 0 && text[0] == '>')
        {
            b_greentext = true;
        }

        if (b_greentext)
        {
            w.bg = COLO.post_bg;
            w.fg = COLO.post_greentext;
        }

        // post num quote
        if (parse_post_num_quote(text) != -1)
        {
            w.b_underline = true;
            w.bg = COLO.post_bg;
            w.fg = COLO.post_reply;
        }

        b_prev_was_newline = false;
    }
}


void TextWidget::append_text(const std::vector<term_word>& words)
{
    for (const auto& w : words)
    {
        term_words.push_back(w);
    }
}


void TextWidget::append_text(const std::wstring& str, bool b_rebuild, bool b_bold, bool b_underline, bool b_reverse, uint32_t bg, uint32_t fg)
{
    std::wstring s = str;
    sanitize_text(s);

    if (bg == -1) bg = bg_color;
    if (fg == -1) fg = fg_color;

    std::vector<term_word> new_words;

    // split text at space chars
    std::wstring sp = str_to_wstring(" ");
    std::vector<std::wstring> words1 = split(s, sp);

    // split splitted words at newline chars
    sp = str_to_wstring("\n");
    for (auto& w : words1)
    {
        std::vector<std::wstring> words2 = split(w, sp);
        int nl = words2.size() - 1;
        for (auto& w2 : words2)
        {
            new_words.emplace_back(
                w2, bg, fg, b_bold, b_underline, b_reverse);
            // add newline chars as separate words
            if (nl > 0)
            {
                term_word nl_w = term_word::newline();
                nl_w.bg = bg;
                nl_w.fg = fg;
                new_words.push_back(nl_w);
                nl--;
            }
        }
    }

    if (b_parse_4chan)
    {
        parse_4chan(new_words);
    }

    for (auto& w : new_words)
    {
        term_words.push_back(w);
    }

    if (b_rebuild)
    {
        rebuild();
    }
}


void TextWidget::append_text(const char* str, bool b_rebuild, bool b_bold, bool b_underline, bool b_reverse, uint32_t bg, uint32_t fg)
{
    std::wstring wstr = str_to_wstring(str);
    append_text(wstr, b_rebuild, b_bold, b_underline, b_reverse, bg, fg);
}


void TextWidget::append_text(const std::string& str, bool b_rebuild, bool b_bold, bool b_underline, bool b_reverse, uint32_t bg, uint32_t fg)
{
    append_text(str.c_str(), b_rebuild, b_bold, b_underline, b_reverse, bg, fg);
}


void TextWidget::append_raw_text(const std::wstring& str, bool b_rebuild, bool b_bold, bool b_underline, bool b_reverse, uint32_t bg, uint32_t fg)
{
    if (bg == -1) bg = bg_color;
    if (fg == -1) fg = fg_color;

    std::vector<term_word> new_words;

    // split words at newline chars
    std::wstring sp = str_to_wstring("\n");
    std::vector<std::wstring> words = split(str, sp);
    int nl = words.size() - 1;
    for (auto& w : words)
    {
        new_words.emplace_back(
            w, bg, fg, b_bold, b_underline, b_reverse);
        // add newline chars as separate words
        if (nl > 0)
        {
            term_word nl_w = term_word::newline();
            nl_w.bg = bg;
            nl_w.fg = fg;
            new_words.push_back(nl_w);
            nl--;
        }
    }

    for (auto& w : new_words)
    {
        term_words.push_back(w);
    }

    if (b_rebuild)
    {
        rebuild();
    }
}


void TextWidget::append_raw_text(const char* str, bool b_rebuild, bool b_bold, bool b_underline, bool b_reverse, uint32_t bg, uint32_t fg)
{
    std::wstring wstr = str_to_wstring(str);
    append_raw_text(wstr, b_rebuild, b_bold, b_underline, b_reverse, bg, fg);
}


void TextWidget::append_raw_text(const std::string& str, bool b_rebuild, bool b_bold, bool b_underline, bool b_reverse, uint32_t bg, uint32_t fg)
{
    append_raw_text(str.c_str(), b_rebuild, b_bold, b_underline, b_reverse, bg, fg);
}


void TextWidget::sanitize_text(std::wstring& str)
{
    str.erase(
        std::remove_if(
            str.begin(),
            str.end(),
            [](char c){
                return (c == '\n' || c == '\r' ||
                        c == '\t' || c == '\v' || c == '\f');
            }),
        str.end());

    // TODO: all of this is super inefficient.
    //       write an efficient sanitization algo,
    //       and strip all HTML properly
    replace_substr(str, "<wbr>", "");
    replace_substr(str, "<b>", "");
    replace_substr(str, "<i>", "");
    replace_substr(str, "</b>", "");
    replace_substr(str, "</i>", "");
    replace_substr(str, "</a>", "");
    replace_substr(str, "</span>", "");
    replace_substr(str, "<span class=\"quote\">", "");
    replace_substr(str, "&gt;", ">");
    replace_substr(str, "&#039;", "'");
    replace_substr(str, "&quot;", "\"");
    replace_substr(str, "<br>", "\n");

    {
        std::wstring pat = str_to_wstring("<a href=");
        std::wstring pat2 = str_to_wstring("\">");
        std::wstring rep = str_to_wstring("");
        size_t index = 0;
        size_t len = 0;
        while(1)
        {
            index = str.find(pat, index);
            if (index == std::string::npos)
            {
                break;
            }

            len = str.find(pat2, index);
            len = len - index + pat2.length();

            str.replace(index, len, rep);
        }
    }
}


std::wstring TextWidget::get_word_at_coord(vector2d coord)
{
    if (coord.x < 0 || coord.y < 0) return std::wstring();

    vector2d abs_off = get_absolute_offset();
    int x = coord.x - abs_off.x;
    int y = coord.y - abs_off.y;
    if (y < cells.size())
    {
        std::vector<tb_cell>& row = cells[y];
        if (x < row.size())
        {
            wchar_t ch = (wchar_t)row[x].ch;
            if (ch != ' ')
            {
                while (ch != ' ' && x > 0)
                {
                    x--;
                    ch = (wchar_t)row[x].ch;
                }

                if (x != 0)
                {
                    x++;
                    ch = (wchar_t)row[x].ch;
                }

                std::wstring word;

                while (ch != ' ' && x < row.size() - 1)
                {
                    word += ch;
                    x++;
                    ch = (wchar_t)row[x].ch;
                }

                if (x == row.size() - 1 && ch != ' ')
                {
                    word += ch;
                }

                return word;
            }
        }
    }

    return std::wstring();
}

