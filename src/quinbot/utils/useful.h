#pragma once

#include <cstdint>
#include <Windows.h>
#include <tuple>
#include <algorithm>
#include <vector>
#include <initializer_list>
#include <sstream>
#include <iomanip>

namespace quinbot
{
namespace util
{
    inline std::tuple<int32_t, int32_t, int32_t> date_today()
    {
        SYSTEMTIME sys_time;
        GetLocalTime(&sys_time);
        return {(int32_t)sys_time.wYear, (int32_t)sys_time.wMonth, (int32_t)sys_time.wDay};
    }

    inline std::string date_format_str( int32_t year, int32_t month, int32_t day, const std::string &delimiter = "-" )
    {
        std::stringstream stream;
        stream << year << delimiter;
        stream << std::setfill('0') << std::setw(2) << month << delimiter;
        stream << std::setfill('0') << std::setw(2) << day;
        return stream.str();
    }

    inline int64_t date_format_int( int32_t year, int32_t month, int32_t day )
    {
        return std::stoll(date_format_str(year, month, day, ""));
    }

    inline std::string date_today_str()
    {
        SYSTEMTIME sys_time;
        std::stringstream stream;
        GetLocalTime(&sys_time);
        stream << sys_time.wYear << "-";
        stream << std::setfill('0') << std::setw(2) << sys_time.wMonth << "-";
        stream << std::setfill('0') << std::setw(2) << sys_time.wDay;
        return stream.str();
    }

    template<typename Type>
    inline bool in( const Type &to_find, const std::vector<Type> &container )
    {
        return std::find(container.begin(), container.end(), to_find) != container.end();
    }

    template<typename Type>
    inline bool in( const Type &to_find, const std::initializer_list<Type> &container )
    {
        return std::find(container.begin(), container.end(), to_find) != container.end();
    }
}
}