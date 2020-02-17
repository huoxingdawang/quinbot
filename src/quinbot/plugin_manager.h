#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include "plugin.h"

#include "cqsdk/event.h"

namespace quinbot
{
namespace plugin
{
    class PluginManager
    {
        using PluginList = std::vector<std::shared_ptr<Plugin>>;
    public:
        PluginManager() = default;
        ~PluginManager() = default;

        inline size_t size() const
        {
            return plugins_.size();
        }

        PluginList::const_iterator find( const std::string &name ) const
        {
            for (auto it = plugins_.cbegin(); it != plugins_.cend(); ++it)
            {
                if ((*it)->name() == name)
                    return it;
            }
            return plugins_.cend();
        }

        PluginList::iterator find( const std::string &name )
        {
            for (auto it = plugins_.begin(); it != plugins_.end(); ++it)
            {
                if ((*it)->name() == name)
                    return it;
            }
            return plugins_.end();
        }

        inline bool contains( const std::string &name ) const
        {
            return find(name) != plugins_.end();
        }

        bool register_plugin( const std::shared_ptr<Plugin> &p_plugin, bool enable_immediately = true )
        {
            if (find(p_plugin->name()) != plugins_.end())
                return false;
            if (enable_immediately)
                p_plugin->set_enabled(true);
            plugins_.push_back(p_plugin);
            return true;
        }

        template<typename PluginType, typename ...ArgsType>
        inline bool register_plugin( bool enable_immediately, ArgsType &&...args )
        {
            return register_plugin(std::make_shared<PluginType>(std::forward<ArgsType>(args)...), enable_immediately);
        }

        inline bool enable( const std::string &name )
        {
            auto it = find(name);
            if (it == plugins_.end())
                return false;
            (*it)->set_enabled(true);
            return true;
        }

        inline bool disable( const std::string &name )
        {
            auto it = find(name);
            if (it == plugins_.end())
                return false;
            (*it)->set_enabled(false);
            return true;
        }

        inline void on_initialize()
        {
            update_callback_(&Plugin::on_initialize);
        }

        inline void on_enable()
        {
            update_callback_(&Plugin::on_enable);
        }

        inline void on_disable()
        {
            update_callback_(&Plugin::on_disable);
        }

        inline void on_coolq_start()
        {
            update_callback_(&Plugin::on_coolq_start);
        }

        inline void on_coolq_exit()
        {
            update_callback_(&Plugin::on_coolq_exit);
        }

#define QUINBOT_EVENT( Name, EventName ) \
inline void __####Name##( const cq::event::##EventName## &e ) { update_callback_(&Plugin::##Name##, e); }

        QUINBOT_EVENT(on_private_msg, PrivateMessageEvent);
        QUINBOT_EVENT(on_group_msg, GroupMessageEvent);
        QUINBOT_EVENT(on_discuss_msg, DiscussMessageEvent);
        QUINBOT_EVENT(on_group_upload, GroupUploadEvent);
        QUINBOT_EVENT(on_group_admin, GroupAdminEvent);
        QUINBOT_EVENT(on_group_member_decrease, GroupMemberDecreaseEvent);
        QUINBOT_EVENT(on_group_member_increase, GroupMemberIncreaseEvent);
        QUINBOT_EVENT(on_group_ban, GroupBanEvent);
        QUINBOT_EVENT(on_friend_add, FriendAddEvent);
        QUINBOT_EVENT(on_friend_request, FriendRequestEvent);
        QUINBOT_EVENT(on_group_request, GroupRequestEvent);

#undef QUINBOT_EVENT

    private:
        PluginList plugins_;

        template<typename FuncType, typename ...ArgsType>
        void update_callback_( FuncType callback, ArgsType &&...args )
        {
            for (const auto &p_plugin : plugins_)
            {
                if (!p_plugin->is_enabled())
                    continue;
                (*p_plugin.*callback)(std::forward<ArgsType>(args)...);
            }
        }
    };
}
}