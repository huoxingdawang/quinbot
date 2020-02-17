#pragma once

#include <string>
#include <vector>
#include <map>

#include <cpr/cpr.h>
#include "../../utils/json.hpp"

#include "../../plugin.h"
#include "../../bot.h"

namespace quinbot
{
namespace plugin
{
    class RepeaterPlugin final : public Plugin
    {
    public:
        RepeaterPlugin()
            :   Plugin("repeater")
        {
        }

        void on_group_msg( const cq::event::GroupMessageEvent &e ) override
        {
            int64_t group_id = e.group_id;
            if (group_last_message_.find(group_id) == group_last_message_.end())
            {
                group_last_message_[group_id] = e.message;
                have_sent_[group_id] = false;
                return;
            }
            if (e.message == group_last_message_[group_id])
            {
                if (!have_sent_[group_id] && bot.get_command_manager()->has_command_prefix(e.raw_message).empty())
                {
                    cq::api::send_group_msg(group_id, e.message);
                    have_sent_[group_id] = true;
                }
            }
            else
            {
                group_last_message_[group_id] = e.message;
                have_sent_[group_id] = false;
            }
        }

    private:
        std::map<int64_t, cq::message::Message> group_last_message_;
        std::map<int64_t, bool> have_sent_;
    };
}
}