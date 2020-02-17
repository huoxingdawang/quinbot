#ifndef __QUINBOT_COMMAND_INFO_H__
#define __QUINBOT_COMMAND_INFO_H__

#include <optional>
#include <string>

#include "command_line.h"
#include "enums.h"

namespace quinbot
{
namespace command
{
    struct CommandInfo
    {
        static const int64_t NOT_A_GROUP_OR_DISCUSS = -0x3f3f3f3f3f;

        int64_t user_id;
        int64_t group_or_discuss_id;
        eMessageType type;
        CommandLine command_line;

        CommandInfo( int64_t user_id, int64_t group_or_discuss_id_, const CommandLine &command_line, eMessageType &type )
        {
            this->user_id = user_id;
            this->group_or_discuss_id = group_or_discuss_id_;
            this->command_line = command_line;
            this->type = type;
        }

        CommandInfo( int64_t user_id, const CommandLine &command_line )
        {
            this->user_id = user_id;
            this->group_or_discuss_id = NOT_A_GROUP_OR_DISCUSS;
            this->command_line = command_line;
            this->type = eMessageType::PRIVATE;
        }

        CommandInfo( int64_t user_id, int64_t group_or_discuss_id_, const CommandLine &command_line, bool is_group = true )
        {
            this->user_id = user_id;
            this->group_or_discuss_id = group_or_discuss_id_;
            this->command_line = command_line;
            this->type = is_group ? eMessageType::GROUP : eMessageType::DISCUSS;
        }

        std::pair<int64_t, int64_t> id()
        {
            return std::make_pair(user_id, group_or_discuss_id);
        }

        inline int64_t get_user_id() const
        {
            return user_id;
        }

        inline int64_t get_group_id() const
        {
            return type == eMessageType::GROUP ? group_or_discuss_id : -1;
        }

        inline int64_t get_discuss_id() const
        {
            return type == eMessageType::DISCUSS ? group_or_discuss_id : -1;
        }

        inline int64_t send_back( const std::string &message ) const
        {
            if (type == eMessageType::PRIVATE)
                return cq::api::send_private_msg(get_user_id(), message);
            else if (type == eMessageType::GROUP)
                return cq::api::send_group_msg(get_group_id(), message);
            else if (type == eMessageType::DISCUSS)
                return cq::api::send_discuss_msg(get_discuss_id(), message);
            return -1;
        }

        inline void send_back_to_private( const std::string &message) const
        {
            cq::api::send_private_msg(get_user_id(), message);
        }

    };
}
}

#endif