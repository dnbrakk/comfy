/**
 * Comfy
 *
 *  Copyright 2019 by Wolfish <wolfish@airmail.cc>
 *  https://wolfish.neocities.org/soft/comfy/
 *
 *  Licensed under the GPL v2.0 only.
 */
#pragma once
#include <string>


static void str_to_lower(std::string& str)
{
    std::transform(str.begin(), str.end(), str.begin(),
        [](unsigned char c){ return std::tolower(c); });
}


static std::wstring str_to_wstring(const char* utf8Bytes)
{
    //setup converter
    using convert_type = std::codecvt_utf8<typename std::wstring::value_type>;
    std::wstring_convert<convert_type, typename std::wstring::value_type> converter;

    //use converter (.to_bytes: wstr->str, .from_bytes: str->wstr)
    return converter.from_bytes(utf8Bytes);
}


static std::wstring str_to_wstring(const std::string& str)
{
    return str_to_wstring(str.c_str());
}


//std::wstring s2ws(const std::string& str)
//{
//    using convert_typeX = std::codecvt_utf8<wchar_t>;
//    std::wstring_convert<convert_typeX, wchar_t> converterX;
//
//    return converterX.from_bytes(str);
//}


static std::string wstr_to_string(const std::wstring& wstr)
{
    using convert_type = std::codecvt_utf8<typename std::wstring::value_type>;
    std::wstring_convert<convert_type, typename std::wstring::value_type> converter;

    return converter.to_bytes(wstr);
}


// str is modified
static void replace_substr(std::string& str, std::string substr, std::string repl)
{
    size_t index = 0;
    for(;;)
    {
        index = str.find(substr, index);
        if (index == std::string::npos)
        {
            break;
        }

        str.replace(index, substr.length(), repl);
    }
}


// str is modified
static void replace_substr(std::wstring& str, char* _substr, char* _repl)
{
    std::wstring substr = str_to_wstring(_substr);
    std::wstring repl = str_to_wstring(_repl);
    size_t index = 0;
    for(;;)
    {
        index = str.find(substr, index);
        if (index == std::string::npos)
        {
            break;
        }

        str.replace(index, substr.length(), repl);
    }
}


template <typename S, typename T>
inline std::vector<S> split(const S& s, T delimiter)
{
    std::vector<S> result;
    if (s.length() == 0)
    {
        result.emplace_back(s);
        return result;
    }

    std::size_t current = 0;
    std::size_t p = s.find_first_of(delimiter, 0);

    while (p != std::string::npos)
    {
        result.emplace_back(s, current, p - current);
        current = p + 1;
        p = s.find_first_of(delimiter, current);
    }

    result.emplace_back(s, current);

    return result;
}


