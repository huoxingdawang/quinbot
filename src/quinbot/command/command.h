#pragma once

#include <string>
#include <functional>
#include <optional>

#include "../enums.h"
#include "command_info.h"
#include "command_line.h"

namespace quinbot
{
namespace command
{
    class Command
    {
    public:
        Command( const std::string &name, eCommandType command_type = eCommandType::UNKNOWN )
            :   name_(name),
                command_type_(command_type),
                is_prefix_command_(false)
        {}

        virtual ~Command() = default;

        virtual eExecuteResult process( const CommandInfo &info )
        {
            return eExecuteResult::NO_CALLBACK;
        }

        inline eExecuteResult execute( const CommandInfo &info )
        {
            return process(info);
        }

        inline std::string name() const
        {
            return name_;
        }

        std::vector<std::string> aliases_list() const
        {
            std::vector<std::string> vec_ret;
            for (const auto &alias : aliases_)
                vec_ret.push_back(alias);
            return vec_ret;
        }

        inline std::unordered_set<std::string> &aliases()
        {
            return aliases_;
        }

        inline const std::unordered_set<std::string> &aliases() const
        {
            return aliases_;
        }

        inline eCommandType type() const
        {
            return command_type_;
        }

        inline bool has_help_message() const
        {
            return !help_message_.empty();
        }

        inline std::string help_message() const
        {
            if (help_message_.empty())
                return u8"该命令无有效的帮助提示信息";
            return help_message_;
        }

        inline void set_help_message( const std::string &help_message )
        {
            help_message_ = help_message;
        }

        inline bool add_alias( const std::string &alias )
        {
            if (is_prefix_command_)
                return false;
            return aliases_.insert(alias).second;
        }

        inline void set_aliases( std::initializer_list<std::string> aliases )
        {
            if (is_prefix_command_)
                return;
            aliases_.clear();
            for (const auto &alias : aliases)
                add_alias(alias);
        }

        inline void set_aliases( const std::vector<std::string> &aliases )
        {
            if (is_prefix_command_)
                return;
            aliases_.clear();
            for (const auto &alias : aliases)
                add_alias(alias);
        }

        void configure( const std::vector<std::string> &aliases, const std::string &help_message )
        {
            set_aliases(aliases);
            set_help_message(help_message);
        }

        inline eExecuteResult operator()( const CommandInfo &info )
        {
            return execute(info);
        }
    
    protected:
        std::string name_;
        std::unordered_set<std::string> aliases_;
        std::string help_message_;
        
        std::unordered_set<std::string> prefixes_call_;

    private:
        eCommandType command_type_;
        bool is_prefix_command_;
    };

    class PublicCommand : public Command
    {
    public:
        PublicCommand( const std::string &name )
            :   Command(name, eCommandType::PUBLIC)
        {}

        virtual ~PublicCommand() = default;
    };

    class PrivateCommand : public Command
    {
    public:
        PrivateCommand( const std::string &name )
            :   Command(name, eCommandType::PRIVATE)
        {}

        virtual ~PrivateCommand() = default;
    };

    class GroupCommand : public Command
    {
        using ProcessCallback = std::function<eExecuteResult ( const CommandInfo & )>;
    public:
        GroupCommand( const std::string &name )
            :   Command(name, eCommandType::GROUP)
        {}

        virtual ~GroupCommand() = default;
    };

    class DiscussCommand : public Command
    {
        using ProcessCallback = std::function<eExecuteResult ( const CommandInfo & )>;
    public:
        DiscussCommand( const std::string &name )
            :   Command(name, eCommandType::DISCUSS)
        {}

        virtual ~DiscussCommand() = default;
    };
}
}
