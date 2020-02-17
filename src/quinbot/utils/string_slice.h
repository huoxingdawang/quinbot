#ifndef __QUINBOT_STRING_SLICE_H__
#define __QUINBOT_STRING_SLICE_H__

#include <string>

namespace quinbot
{
namespace util
{
    class StringSlice
    {
    public:
        StringSlice( const std::string::const_iterator &left, const std::string::const_iterator &right )
            :   left_(left),
                right_(right)
        {}

        StringSlice( const std::string &str, std::string::size_type left, std::string::size_type right )
        {
            left_ = str.cbegin(); left_ += left;
            right_ = str.cbegin(); right_ += right;
        }

        inline std::string to_string() const
        {
            return std::string(left_, right_);
        }

    private:
        typename std::string::const_iterator left_;
        typename std::string::const_iterator right_;
    };
}
}

#endif