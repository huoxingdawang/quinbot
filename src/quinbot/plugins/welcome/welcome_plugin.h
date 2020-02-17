#pragma once

#include "../../plugin.h"
#include "../../bot.h"

namespace quinbot
{
namespace plugin
{
    class WelcomePlugin final : public Plugin
    {
    public:
        WelcomePlugin()
            :   Plugin("welcome")
        {}

        ~WelcomePlugin() = default;

        void on_group_member_increase( const cq::event::GroupMemberIncreaseEvent &e ) 
        {
            cq::message::Message msg;
            msg += cq::message::MessageSegment::at(e.user_id);
            msg += cq::message::MessageSegment::text("\n停一下pong友，你有香香鸡吗");
            cq::api::send_group_msg(e.group_id, msg);
        }
        
    private:

    };
}
}