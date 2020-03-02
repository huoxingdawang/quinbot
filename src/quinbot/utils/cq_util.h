#pragma once

#include <string>
#include <cstdint>

#include "../cqsdk/message.h"

namespace quinbot
{
namespace util
{
    class MessageBuilder
    {
        using MS = cq::message::MessageSegment;
    public:
        MessageBuilder( const std::string &str = "" ) { message_ += MS::text(str); }
        inline MessageBuilder &text( const std::string &str ) { message_ += MS::text(str); return *this; }
        inline MessageBuilder &emoji( uint32_t id ) { message_ += MS::emoji(id); return *this; }
        inline MessageBuilder &face( int id ) { message_ += MS::face(id); return *this; }
        inline MessageBuilder &image( const std::string &file ) { message_ += MS::image(file); return *this; }
        inline MessageBuilder &record( const std::string &file, const bool magic = false ) { message_ += MS::record(file, magic); return *this; }
        inline MessageBuilder &at( int64_t user_id ) { message_ += MS::at(user_id); return *this; }
        inline MessageBuilder &rps() { message_ += MS::rps(); return *this; }
        inline MessageBuilder &dice() { message_ += MS::dice(); return *this; }
        inline MessageBuilder &shake() { message_ += MS::shake(); return *this; }
        inline MessageBuilder &anonymous ( bool ignore_failure = false ) { message_ += MS::anonymous(ignore_failure); return *this; }

        inline MessageBuilder &share( const std::string &url, const std::string &title, const std::string &content = "", const std::string &image_url = "" )
        { 
            message_ += MS::share(url, title, content, image_url);
            return *this;
        }

        inline MessageBuilder &contact( MS::ContactType &type, int64_t id )
        {
            message_ += MS::contact(type, id);
            return *this;
        }

        inline MessageBuilder &location( const double latitude, const double longitude, const std::string &title = "", const std::string &content = "" )
        {
            message_ += MS::location(latitude, longitude, title, content);
            return *this;
        }

        inline MessageBuilder &music( const std::string &type, const int64_t id ) { message_ += MS::music(type, id); return *this; }
        inline MessageBuilder &music( const std::string &type, const int64_t id, const int32_t style ) { message_ += MS::music(type, id, style); return *this; }

        inline MessageBuilder &music( const std::string &url, const std::string &audio_url, const std::string &title, const std::string &content = "", const std::string &image_url = "" )
        {
            message_ += MS::music(url, audio_url, title, content, image_url);
            return *this;
        }

        inline std::string str() const
        {
            return std::to_string(message_);
        }

        inline cq::message::Message message() const
        {
            return message_;
        }

        inline operator std::string() const
        {
            return str();
        }

    private:
        cq::message::Message message_;
    };
}
}
