#pragma once

#include <cstdint>
#include <string>

namespace quinbot
{
namespace util
{
    class Datetime
    {
    public:
        int8_t year, month, day;
        int8_t hour, minute, second, millisecond;

        Datetime( int8_t year, int8_t month, int8_t day, int8_t hour, int8_t minute, int8_t second, int8_t millisecond )
        {
            this->year = year;
            this->month = month;
            this->day = day;
            this->hour = hour;
            this->minute = minute;
            this->second = second;
            this->millisecond = millisecond;
        }

        Datetime( int8_t hour, int8_t minute, int8_t second, int8_t millisecond = 0 )
        {
            this->hour = hour;
            this->minute = minute;
            this->second = second;
        }

        Datetime &operator=( const Datetime &other ) = default;

        bool operator==( const Datetime &other ) const
        {
            return  year == other.year &&
                    month == other.month &&
                    day == other.day &&
                    hour == other.hour &&
                    minute == other.minute &&
                    second == other.second &&
                    millisecond == other.millisecond;
        }

        bool operator!=( const Datetime &other ) const
        {
            return !operator==(other);
        }

        //std::string str() const;

    private:
        
    };
}
}