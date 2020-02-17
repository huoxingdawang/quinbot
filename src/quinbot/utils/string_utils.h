#pragma once

#include <string>
#include <vector>
#include <regex>
#include <sstream>
#include <iomanip>

namespace quinbot
{
namespace util
{
    void split( std::vector<std::string> &result, const std::string &str, const std::string &delimiter, bool need_to_clear = true );
    std::vector<std::string> split( const std::string &str, const std::string &delimiter );
    
    void split_with_regex( std::vector<std::string> &result, const std::string &str, const std::string &regex_str );
    std::vector<std::string> split_with_regex( const std::string &str, const std::string &regex_str );

    inline bool starts_with( const std::string &str, const std::string &prefix )
    {
        return str.compare(0, prefix.size(), prefix) == 0;
    }

    std::string starts_with_multi( const std::string &str, const std::vector<std::string> &prefixes );

    inline bool ends_with( const std::string &str, const std::string &suffix )
    {
        if (str.size() < suffix.size())
            return false;
        return str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
    }

    int32_t count_str( const std::string &str, const std::string &to_count );

    bool is_any_of( char ch, const std::string &str_set );

    std::string lstrip_str( const std::string &str, const std::string &to_strip );
    std::string rstrip_str( const std::string &str, const std::string &to_strip );
    std::string strip_str( const std::string &str, const std::string &to_strip );

    inline char ascill_to_lower( const char ch ) { return ch + 32; }
    inline char ascill_to_upper( const char ch ) { return ch - 32; }
    std::string ascill_to_lower( const std::string &str );
    std::string ascill_to_upper( const std::string &str );

    bool str_equals( const std::string &str, std::string::const_iterator start, const std::string &to_compare );
    bool str_equals( const std::string &str, std::string::size_type start, const std::string &to_compare );

    std::string wstring_to_string( const std::wstring &str );
    std::wstring string_to_wstring( const std::string &str );

    std::string get_current_time_str();
}
}