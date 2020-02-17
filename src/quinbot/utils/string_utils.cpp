#include "string_utils.h"

#include <Windows.h>


namespace quinbot
{
namespace util
{
    void split( std::vector<std::string> &result, const std::string &str, const std::string &delimiter, bool need_to_clear )
    {
        if (need_to_clear)
            result.clear();
        if (!str.size())
            return;
        if (!delimiter.size())
        {
            result.push_back(delimiter);
            return;
        }
        typename std::string::size_type p1, p2;
        p1 = 0, p2 = str.find(delimiter);
        while (p2 != std::string::npos)
        {
            if (p1 != p2)
                result.emplace_back(str, p1, p2 - p1);
            p1 = p2 + delimiter.size();
            p2 = str.find(delimiter, p1);
        }
        if (p1 != str.length())
            result.emplace_back(str, p1, str.size() - p1);
    }

    std::vector<std::string> split( const std::string &str, const std::string &delimiter )
    {
        std::vector<std::string> result;
        split(result, str, delimiter);
        return result;
    }

    void split_with_regex( std::vector<std::string> &result, const std::string &str, const std::string &regex_str )
    {
        std::regex re = std::regex(regex_str);
        std::sregex_token_iterator re_begin = std::sregex_token_iterator(str.begin(), str.end(), re, -1);
        std::sregex_token_iterator re_end = std::sregex_token_iterator();

        result.clear();

        for (std::sregex_token_iterator it = re_begin; it != re_end; ++it)
        {
            if (it->str().empty())
                continue;
            result.push_back(it->str());
        }
    }
    
    std::vector<std::string> split_with_regex( const std::string &str, const std::string &regex_str )
    {
        std::vector<std::string> result;
        split_with_regex(result, str, regex_str);
        return result;
    }

    std::string starts_with_multi( const std::string &str, const std::vector<std::string> &prefixes )
    {
        for (const auto &prefix : prefixes)
        {
            if (starts_with(str, prefix))
                return prefix;
        }
        return "";
    }

    int32_t count_str( const std::string &str, const std::string &target_str )
    {
        std::string::size_type pos1 = 0;
        std::string::size_type pos2 = str.find(target_str);
        int32_t count = 0;
        while (pos2 != std::string::npos)
        {
            if (pos1 != pos2)
                ++count;
            pos1 = pos2 + target_str.size();
            pos2 = str.find(target_str, pos1);
        }
        return count;
    }

    bool is_any_of( char ch , const std::string &str_set )
    {
        return str_set.find(ch) != std::string::npos;
    }

    std::string lstrip_str( const std::string &str, const std::string &to_strip )
    {
        std::string::size_type str_size = str.size();
        std::string::size_type pos = 0;
        while (pos != str_size && is_any_of(str[pos], to_strip))
            ++pos;
        return std::string(str, pos, str_size - pos);
    }

    std::string rstrip_str( const std::string &str, const std::string &to_strip )
    {
        std::string::size_type str_size = str.size();
        std::string::size_type pos = str_size;
        while (pos != 0 && is_any_of(str[pos], to_strip))
            --pos;
        return std::string(str, 0, pos);
    }

    std::string strip_str( const std::string &str, const std::string &to_strip )
    {
        std::string::size_type str_size = str.size();
        std::string::size_type pos1 = 0;
        std::string::size_type pos2 = str_size;
        while (pos1 != str_size && is_any_of(str[pos1], to_strip))
            ++pos1;
        while (pos2 != 0 && is_any_of(str[pos2], to_strip))
            --pos2;
        return std::string(str, pos1, pos2 - pos1);
    }

    std::string ascill_to_lower( const std::string &str )
    {
        std::string lower_str;
        for (std::string::size_type i = 0; str[i]; ++i)
        {
            if (str[i] >= 'A' && str[i] <= 'Z')
                lower_str += ascill_to_lower(str[i]);
            else
                lower_str += str[i];
        }
        return lower_str;
    }

    std::string ascill_to_upper( const std::string &str )
    {
        std::string upper_str;
        for (std::string::size_type i = 0; str[i]; ++i)
        {
            if (str[i] >= 'a' && str[i] <= 'z')
                upper_str += ascill_to_upper(str[i]);
            else
                upper_str += str[i];
        }
        return upper_str;
    }

    bool str_equals( const std::string &str, std::string::const_iterator start, const std::string &to_compare )
    {
        std::string::const_iterator it1 = start, end1 = str.end();
        std::string::const_iterator it2 = to_compare.begin(), end2 = to_compare.end();
        for (; it1 != end1 && it2 != end2; ++it1, ++it2)
        {
            if ((*it1) != (*it2))
                return false;
        }
        if (it2 != end2)
            return false;
        return true;
    }
    
    bool str_equals( const std::string &str, std::string::size_type start, const std::string &to_compare )
    {
        return str_equals(str, str.cbegin() + start, to_compare);
    }

    std::string wstring_to_string( const std::wstring &wstr )
    {
        size_t str_len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), NULL, 0, NULL, NULL);
        char *buffer = new char[str_len + 1];
        WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), buffer, str_len, NULL, NULL);
        buffer[str_len] = '\0';
        std::string result(buffer, str_len);
        delete[] buffer;
        return result;
    }

    std::wstring string_to_wstring( const std::string &str )
    {
        size_t wstr_len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), NULL, 0);
        wchar_t *buffer = new wchar_t[wstr_len + 1];
        MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), buffer, wstr_len);
        buffer[wstr_len] = '\0';
        std::wstring result(buffer, wstr_len);
        delete[] buffer;
        return result; 
    }

    std::string get_current_time_str()
    {
        SYSTEMTIME sys_time;
        std::stringstream stream;
        GetLocalTime(&sys_time);
        stream << sys_time.wYear << "/" << sys_time.wMonth << "/" << sys_time.wDay << " ";
        stream << std::setfill('0') << std::setw(2) << sys_time.wHour << ":";
        stream << std::setfill('0') << std::setw(2) << sys_time.wMinute << ":";
        stream << std::setfill('0') << std::setw(2) << sys_time.wSecond;
        return stream.str();
    }
}
}