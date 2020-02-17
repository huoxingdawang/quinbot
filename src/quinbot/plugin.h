#pragma once

#include <vector>
#include <string>

#include "cqsdk/event.h"

#include "command/arg_parser.h"
#include "command/command.h"
#include "command/command_info.h"
#include "command/command_line.h"
#include "command/command_manager.h"
#include "utils/database.h"

namespace quinbot
{
namespace plugin
{
    class Plugin
    {
    public:
        Plugin( const std::string &name, bool enable_by_default = false )
            :   name_(name),
                enabled_(enable_by_default)
        { 
        }


        virtual ~Plugin() = default;

        inline std::string name() const
        {
            return name_;
        }

        inline bool is_enabled() const
        {
            return enabled_;
        }

        inline void set_enabled( bool enabled )
        {
            enabled_ = enabled;
        }
    
        virtual void on_initialize() {}
        virtual void on_enable() {}
        virtual void on_disable() {}
        virtual void on_coolq_start() {}
        virtual void on_coolq_exit() {}

        virtual void on_private_msg( const cq::event::PrivateMessageEvent &e ) {}
        virtual void on_group_msg( const cq::event::GroupMessageEvent &e ) {}
        virtual void on_discuss_msg( const cq::event::DiscussMessageEvent &e ) {}

        virtual void on_group_upload( const cq::event::GroupUploadEvent &e ) {}
        virtual void on_group_admin( const cq::event::GroupAdminEvent &e ) {}
        virtual void on_group_member_decrease( const cq::event::GroupMemberDecreaseEvent &e ) {}
        virtual void on_group_member_increase( const cq::event::GroupMemberIncreaseEvent &e ) {}
        virtual void on_group_ban( const cq::event::GroupBanEvent &e ) {}
        virtual void on_friend_add( const cq::event::FriendAddEvent &e ) {}
        virtual void on_friend_request( const cq::event::FriendRequestEvent &e ) {}
        virtual void on_group_request( const cq::event::GroupRequestEvent &e ) {}


    private:
    
    protected:
        std::string name_;
        bool enabled_;
    };
}
}