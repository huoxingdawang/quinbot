#ifndef __QUINBOT_COMMAND_MANAGER_H__
#define __QUINBOT_COMMAND_MANAGER_H__

#include <initializer_list>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <set>
#include <vector>
#include <memory>

#include "../../cqsdk/logging.h"

#include "command.h"
#include "command_line.h"
#include "command_info.h"
#include "utils/string_utils.h"

namespace quinbot
{
namespace command
{
    class CommandManager
    {
        template<typename CommandType>
        using CommandPointer = std::shared_ptr<CommandType>;

        template<typename CommandType>
        using StringCommandPair = std::pair<std::string, CommandType>;

        template<typename CommandType>
        using CommandMatcher = std::unordered_map<std::string, CommandPointer<CommandType>>;

        using StringSet = std::unordered_set<std::string>;
        using AliasMatcher = std::unordered_map<std::string, std::string>;
    
    public:
        CommandManager()
        {
        }

        CommandManager( std::initializer_list<std::string> prefixes )
        {
            for (const auto &prefix : prefixes)
                prefixes_.insert(prefix);
        }

        ~CommandManager()
        {
            cq::logging::debug("fuck me", "");
        }

        eCommandType type_of( const std::string &name ) const
        {
            if (public_command_matcher_.find(name) != public_command_matcher_.end())
                return eCommandType::PUBLIC;
            if (private_command_matcher_.find(name) != private_command_matcher_.end())
                return eCommandType::PRIVATE;
            if (group_command_matcher_.find(name) != group_command_matcher_.end())
                return eCommandType::GROUP;
            if (discuss_command_matcher_.find(name) != discuss_command_matcher_.end())
                return eCommandType::DISCUSS;
            return eCommandType::UNKNOWN;
        }

        eCommandType alias_type_of( const std::string &alias ) const
        {
            if (public_alias_matcher_.find(alias) != public_alias_matcher_.end())
                return eCommandType::PUBLIC;
            if (private_alias_matcher_.find(alias) != private_alias_matcher_.end())
                return eCommandType::PRIVATE;
            if (group_alias_matcher_.find(alias) != group_alias_matcher_.end())
                return eCommandType::GROUP;
            if (discuss_alias_matcher_.find(alias) != discuss_alias_matcher_.end())
                return eCommandType::DISCUSS;
            return eCommandType::UNKNOWN;
        }

        inline bool contains( const std::string &name) const
        {
            return type_of(name) != eCommandType::UNKNOWN;
        }

        inline std::string alias_contains( const std::string &alias ) const
        {
            eCommandType type = alias_type_of(alias);
            if (!allow_aliases_ || type == eCommandType::UNKNOWN)
                return "";
            return alias_matcher_[static_cast<size_t>(type) - 1].at(alias);
        }

        std::vector<std::string> get_all_help_queryable_commands() const
        {
            std::vector<std::string> result;
            for (int matcher_index = 0; matcher_index < 4; ++matcher_index)
            {
                for (const auto &command_pair : command_matcher_[matcher_index])
                {
                    const auto &[name, ptr] = command_pair;
                    std::string format_str;
                    if (ptr->has_help_message())
                    {
                        format_str.append(name); 
                        if (ptr->aliases().size() != 0)
                        {
                            format_str += '(';
                            const auto &aliases = ptr->aliases();
                            for (auto it = aliases.begin(); it != aliases.end(); ++it)
                            {
                                format_str += (*it);
                                format_str += '/';
                            }
                            format_str.back() = ')';
                        }
                        result.push_back(format_str);
                    }    
                }
            }
            return result;
        }

        inline size_t size() const
        {
            return  public_command_matcher_.size() +
                    private_command_matcher_.size() +
                    group_command_matcher_.size() +
                    discuss_command_matcher_.size(); 
        }

        inline size_t size( eCommandType type ) const
        {
            switch(type)
            {
                case eCommandType::UNKNOWN:
                    return 0;
                case eCommandType::PUBLIC:
                    return public_command_matcher_.size();
                case eCommandType::PRIVATE:
                    return private_command_matcher_.size();
                case eCommandType::GROUP:
                    return group_command_matcher_.size();
                case eCommandType::DISCUSS:
                    return discuss_command_matcher_.size();
            }
        }

        inline bool is_allow_aliases() const
        {
            return allow_aliases_;
        }

        inline bool is_case_sensitive() const
        {
            return case_sensitive_;
        }

        inline bool add_prefix( const std::string &prefix )
        {
            if (!case_sensitive_)
                return prefixes_.insert(util::ascill_to_lower(prefix)).second;
            return prefixes_.insert(prefix).second;
        }

        void add_prefixes( std::initializer_list<std::string> prefixes )
        {
            for (const auto &prefix : prefixes)
                add_prefix(prefix);
        }

        void add_prefixes( const std::vector<std::string> &prefixes )
        {
            for (const auto &prefix : prefixes)
                add_prefix(prefix);
        }

        inline bool remove_prefix( const std::string &prefix )
        {
            std::string prefix_copy(prefix);
            if (!case_sensitive_)
                prefix_copy = util::ascill_to_lower(prefix_copy);
            if (prefixes_.find(prefix_copy) == prefixes_.end())
                return false;
            prefixes_.erase(prefix_copy);
            return true;
        }

        void clear_prefix()
        {
            prefixes_.clear();
        }

        void set_prefixes( std::initializer_list<std::string> prefixes )
        {
            clear_prefix();
            for (const auto &prefix : prefixes)
                add_prefix(prefix);
        }

        void set_prefixes( const std::vector<std::string> &prefixes )
        {
            clear_prefix();
            for (const auto &prefix : prefixes)
                add_prefix(prefix);
        }

        inline void set_allow_aliases( bool state )
        {
            allow_aliases_ = state;
        }

        inline void set_case_sensitive( bool state )
        {
            case_sensitive_ = state;
        }

        template<typename CommandType, typename ...ArgsType>
        inline void register_command( ArgsType &&...args )
        {
            do_register_command_(std::make_shared<CommandType>(std::forward<ArgsType>(args)...));
        }

        template<typename CommandPointerType>
        inline void register_command( const std::shared_ptr<CommandPointerType> &p_command )
        {
            do_register_command_(p_command);
        }

        template<typename ...CommandPointerTypes>
        inline void register_commands( CommandPointerTypes &&...ptrs )
        {
            do_register_commands_(std::forward<CommandPointerTypes>(ptrs)...);
        }

        bool remove_command( const std::string &name, eCommandType type )
        {
            bool result = false;
            cq::logging::debug("remove_v2", std::to_string(name) + " " + std::to_string((int)type));
            switch (type)
            {
                case eCommandType::UNKNOWN:
                    break;
                case eCommandType::PUBLIC:
                    result = try_to_remove_command_(public_command_matcher_, name);
                    break;
                case eCommandType::PRIVATE:
                    result = try_to_remove_command_(private_command_matcher_, name);
                    break;
                case eCommandType::GROUP:
                    result = try_to_remove_command_(group_command_matcher_, name);
                    break;
                case eCommandType::DISCUSS:
                    result = try_to_remove_command_(discuss_command_matcher_, name);
                    break;
            }
            if (allow_aliases_ && result)
            {
                for (const auto &alias : get(name)->aliases())
                    do_remove_alias_(alias, type);
            }
            return result;
        }

        bool remove_command( const std::string &name )
        {
            eCommandType type;
            type = type_of(name);
            cq::logging::debug("remove", name + "  " + std::to_string((int)type));
            if (type == eCommandType::UNKNOWN)
                return false;
            switch(type)
            {
                case eCommandType::PUBLIC:
                    public_command_matcher_.erase(name);
                    break;
                case eCommandType::PRIVATE:
                    private_command_matcher_.erase(name);
                    break;
                case eCommandType::GROUP:
                    group_command_matcher_.erase(name);
                    break;
                case eCommandType::DISCUSS:
                    discuss_command_matcher_.erase(name);
                    break;
            }
            if (allow_aliases_)
            {
                for (const auto &alias : get(name)->aliases())
                    do_remove_alias_(alias, type);
            }
            return true;
        }

        inline CommandPointer<Command> get( const std::string &name, eCommandType type ) const
        {
            if (type == eCommandType::UNKNOWN)
                return nullptr;
            if (type == eCommandType::PUBLIC)
                return try_to_get_(public_command_matcher_, name);
            if (type == eCommandType::PRIVATE)
                return try_to_get_(private_command_matcher_, name);
            if (type == eCommandType::GROUP)
                return try_to_get_(group_command_matcher_, name);
            if (type == eCommandType::DISCUSS)
                return try_to_get_(discuss_command_matcher_, name);
            return nullptr;
        }

        inline CommandPointer<Command> get( const std::string &name ) const
        {
            eCommandType type;
            type = type_of(name);
            return get(name, type);
        }

        inline std::pair<bool, std::string> is_command( const std::string &command_str ) const
        {
            std::string prefix = has_command_prefix(command_str);
            if (!prefix.empty())
                return {true, prefix};
            return {false, ""};
        }

        bool is_command( const std::string &command_str, std::string &prefix_out ) const
        {
            prefix_out = has_command_prefix(command_str);
            return !prefix_out.empty();
        }

        std::string prefix_of( const std::string &command_name ) const
        {
            for (const auto &prefix : prefixes_)
            {
                if (util::starts_with(command_name, prefix))
                    return prefix;
            }
            return "";
        }

        inline std::string has_command_prefix( const std::string &command_str ) const
        {
            return prefix_of(command_str);
        }

        eExecuteResult run( const CommandInfo &info )
        {
            auto p_command = get(info.command_line.name());
            if (p_command)
                return p_command->execute(info);
            return eExecuteResult::NOT_A_COMMAND;
        }

    private:
        CommandMatcher<Command> command_matcher_[4];
        CommandMatcher<Command> &public_command_matcher_ = command_matcher_[0];
        CommandMatcher<Command> &private_command_matcher_ = command_matcher_[1];
        CommandMatcher<Command> &group_command_matcher_ = command_matcher_[2];
        CommandMatcher<Command> &discuss_command_matcher_ = command_matcher_[3];

        CommandMatcher<Command> prefix_command_matcher_;

        AliasMatcher alias_matcher_[4];
        AliasMatcher &public_alias_matcher_ = alias_matcher_[0];
        AliasMatcher &private_alias_matcher_ = alias_matcher_[1];
        AliasMatcher &group_alias_matcher_ = alias_matcher_[2];
        AliasMatcher &discuss_alias_matcher_ = alias_matcher_[3];

        bool allow_aliases_;
        bool case_sensitive_;

        std::unordered_set<std::string> prefixes_;
        
        inline std::string strrule_( const std::string &str ) const
        {
            if (case_sensitive_)
                return str;
            return util::ascill_to_lower(str);
        }

        bool do_remove_alias_( const std::string &alias, const eCommandType type = eCommandType::PUBLIC )
        {
            size_t matcher_idx = static_cast<size_t>(type) - 1;
            AliasMatcher &matcher = alias_matcher_[matcher_idx];
            if (matcher.find(alias) != matcher.end())
            {
                matcher.erase(alias);
                return true;
            }
            return false;
        }

        void do_register_command_( const std::shared_ptr<PublicCommand> & ptr )
        {
            public_command_matcher_.insert(std::make_pair(ptr->name(), ptr));
            for (const auto &alias : ptr->aliases())
            {           
                public_alias_matcher_.insert({
                    strrule_(alias),
                    strrule_(ptr->name())
                });
            }
        }

        void do_register_command_( const std::shared_ptr<PrivateCommand> & ptr )
        {
            private_command_matcher_.insert(std::make_pair(ptr->name(), ptr));
            for (const auto &alias : ptr->aliases())
            {
                private_alias_matcher_.insert({
                    strrule_(alias),
                    strrule_(ptr->name())
                });
            }
        }

        void do_register_command_( const std::shared_ptr<GroupCommand> & ptr )
        {
            group_command_matcher_.insert(std::make_pair(ptr->name(), ptr));
            for (const auto &alias : ptr->aliases())
            {
                group_alias_matcher_.insert({
                    strrule_(alias),
                    strrule_(ptr->name())
                });
            }
        }

        void do_register_command_( const std::shared_ptr<DiscussCommand> & ptr )
        {
            discuss_command_matcher_.insert(std::make_pair(ptr->name(), ptr));
            for (const auto &alias : ptr->aliases())
            {
                discuss_alias_matcher_.insert({
                    strrule_(alias),
                    strrule_(ptr->name())
                });
            }
        }

        template<typename CommandPointerType>
        void do_register_commands_( CommandPointerType &&ptr )
        {
            do_register_command_(ptr);
        }

        template<typename CommandPointerType, typename CommandPointerTypes>
        void do_register_commands_( CommandPointerType &&ptr, CommandPointerTypes &&ptrs )
        {
            do_register_command_(ptr);
            do_register_commands_(std::forward<CommandPointerTypes>(ptrs)...);
        }

        template<typename CommandType>
        bool try_to_remove_command_( CommandMatcher<CommandType> &matcher, const std::string &name )
        {
            if (matcher.find(name) == matcher.end())
                return false;
            matcher.erase(name);
            return true;
        }

        template<typename CommandType>
        inline const CommandPointer<CommandType> try_to_get_( CommandMatcher<CommandType> &matcher, const std::string &name ) const
        {
            if (matcher.find(name) != matcher.end())
                return matcher.at(name);
            return nullptr;
        }
    };

}
}

#endif