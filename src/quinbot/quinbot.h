#pragma once

#include <algorithm>
#include <iterator>
#include <memory>
#include <string>

#include "bot.h"
#include "config.h"
#include "enums.h"
#include "logger.h"
#include "command/builtin_command.h"
#include "command/command.h"
#include "command/command_info.h"
#include "command/command_line.h"
#include "command/command_manager.h"
#include "command/command_parser.h"
#include "command/command_processer.h"
#include "utils/json.hpp"
#include "utils/string_slice.h"
#include "utils/string_utils.h"
#include "utils/file_utils.h"
#include "plugin_manager.h"

namespace quinbot
{
    inline void load( const std::shared_ptr<plugin::Plugin> &p_plugin, bool enable_immediately = false )
    {
        bot.get_plugin_manager()->register_plugin(p_plugin, enable_immediately);
    }

    template<typename PluginType, typename ...ArgsType>
    inline void load( bool enable_immediately, ArgsType &&...args )
    {
        bot.get_plugin_manager()->register_plugin<PluginType>(enable_immediately, std::forward<ArgsType>(args)...);
    }

    bool app_allow_on_private_msg_( const cq::event::PrivateMessageEvent &e )
    {
        auto &config = quinbot::bot.get_config();
        if (config.developing)
        {
            if (e.user_id != config.superuser) { return false; }
            else { return true; }
        }
        for (const auto id : config.users)
            if (e.user_id == id)
                return true;
        return false;
    }

    bool app_allow_on_group_msg_( const cq::event::GroupMessageEvent &e )
    {
        auto &config = quinbot::bot.get_config();
        if (config.developing)
        {
            if (e.user_id != config.superuser) { return false; }
            else { return true; }
        }
        for (const auto id : config.groups)
            if (e.group_id == id)
                return true;
        return false;
    }

    bool app_allow_on_discuss_msg_( const cq::event::DiscussMessageEvent &e )
    {
        auto &config = quinbot::bot.get_config();
        if (config.developing)
        {
            if (e.user_id != config.superuser) { return false; }
            else { return true; }
        }
        for (const auto id : config.discussion_groups)
            if (e.discuss_id == id)
                return true;
        return false;
    }

    void initialize()
    {

#pragma region Hook Lifecycle

        cq::app::on_initialize = []()
        {
            quinbot::bot.on_initialize();
            quinbot::bot.get_plugin_manager()->on_initialize();
        };

        cq::app::on_coolq_start = []()
        {
            quinbot::bot.on_coolq_start();
            quinbot::bot.get_plugin_manager()->on_coolq_start();
        };

        cq::app::on_enable = []()
        {
            quinbot::bot.on_enable();
            quinbot::bot.get_plugin_manager()->on_enable();
        };

        cq::app::on_disable = []()
        {
            quinbot::bot.get_plugin_manager()->on_disable();
            quinbot::bot.on_disable();
        };

        cq::app::on_coolq_exit = []()
        {
            quinbot::bot.on_coolq_exit();
            quinbot::bot.get_plugin_manager()->on_coolq_exit();
        };

#pragma endregion

#pragma region Hook Events

#define QUINBOT_HOOK_MESSAGE_EVENT( Name, EventName ) \
cq::event::##Name## = [](const cq::event::##EventName## &e) \
{ \
    if (!app_allow_##Name##_(e)) \
        return; \
    quinbot::bot.command_process(e); \
    quinbot::bot.get_plugin_manager()->__##Name##(e); \
    quinbot::bot.get_logger().debug(u8"接收消息", e.message); \
}

        QUINBOT_HOOK_MESSAGE_EVENT(on_private_msg, PrivateMessageEvent);
        QUINBOT_HOOK_MESSAGE_EVENT(on_group_msg, GroupMessageEvent);
        QUINBOT_HOOK_MESSAGE_EVENT(on_discuss_msg, DiscussMessageEvent);

#undef QUINBOT_HOOK_MESSAGE_EVENT


#define QUINBOT_HOOK_EVENT( Name, EventName ) \
cq::event::##Name## = [](const cq::event::##EventName## &e) \
{ \
    quinbot::bot.get_plugin_manager()->__##Name##(e); \
}

        QUINBOT_HOOK_EVENT(on_group_upload, GroupUploadEvent);
        QUINBOT_HOOK_EVENT(on_group_admin, GroupAdminEvent);
        QUINBOT_HOOK_EVENT(on_group_member_decrease, GroupMemberDecreaseEvent);
        QUINBOT_HOOK_EVENT(on_group_member_increase, GroupMemberIncreaseEvent);
        QUINBOT_HOOK_EVENT(on_group_ban, GroupBanEvent);
        QUINBOT_HOOK_EVENT(on_friend_add, FriendAddEvent);
        QUINBOT_HOOK_EVENT(on_friend_request, FriendRequestEvent);
        QUINBOT_HOOK_EVENT(on_group_request, GroupRequestEvent);

#undef QUINBOT_HOOK_EVENT

#pragma endregion
    }
}