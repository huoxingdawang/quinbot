#pragma once

#include <exception>
#include <string>

namespace quinbot
{
namespace exception
{
    class ArgsParseError final : public std::exception
    {
    public:
        ArgsParseError( const std::string &message )
            :   message_(message)
        {}

        const char *what() const
        {
            return message_.c_str();
        }

    private:
        std::string message_;
    };
}
}