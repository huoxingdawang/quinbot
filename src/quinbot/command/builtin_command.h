#pragma once

#include "command.h"
#include "command_info.h"
#include "command_manager.h"

#include "../cqsdk/cqsdk.h"

namespace quinbot
{
namespace command
{
namespace builtin
{
    class BuiltinCommand_Test final : public PublicCommand
    {
    public:
        BuiltinCommand_Test()
            :   PublicCommand("test")
        {
            set_help_message("这是一个测试命令");
        }

        eExecuteResult process( const CommandInfo &info ) override
        {
            const CommandLine &cl = info.command_line;
            switch (info.type)
            {
                case eMessageType::PRIVATE:
                    info.send_back("Hello, private");
                    break;
                case eMessageType::GROUP:
                    info.send_back("Hello, group");
                    break;
                case eMessageType::DISCUSS:
                    info.send_back("Hello, discuss");
                    break;
            }
            return eExecuteResult::SUCCESS;
        }

        ~BuiltinCommand_Test() = default;
    private:

    };

    class BuiltinCommand_Help final : public PublicCommand
    {
    public:
        BuiltinCommand_Help( std::shared_ptr<CommandManager> &command_manager )
            :   PublicCommand("help"),
                command_manager_(command_manager)
        {
            configure(
                {"帮助", "查询命令帮助"},
                "格式: help [Keyword(可选) command/命令]\n"
                "command为空时列出所有可查命令"
            );
        }

        eExecuteResult process( const CommandInfo &info ) override
        {
            const CommandLine &cl = info.command_line;
            command::ArgsMap args = arg_parse(cl,
                {{"command", {"命令"}}},
                {}
            );

            if (args.is_bad())
            {
                info.send_back(args.error_message);
                return eExecuteResult::USER_ERROR;
            }

            std::string command_name;
            try {
                command_name = args.get<std::string>("command", "", false);
            } catch ( const exception::ArgsParseError &e ) {
                info.send_back(e.what());
                return eExecuteResult::USER_ERROR;
            }

            if (command_name.empty())
            {
                std::string help;
                auto strs = command_manager_->get_all_help_queryable_commands();
                for (const auto &str : strs)
                    help += str + '\n';
                help.pop_back();
                info.send_back(help);
                return eExecuteResult::SUCCESS;
            }

            std::string help;
            if (command_manager_->contains(command_name))
                help = command_manager_->get(command_name)->help_message();
            else
            {
                std::string cname = command_manager_->alias_contains(command_name);
                if (!cname.empty())
                    help = command_manager_->get(cname)->help_message();
                else
                    help = "找不到命令哒";
            }
            info.send_back(help);
            return eExecuteResult::SUCCESS;
        }

        ~BuiltinCommand_Help() = default;

    private:
        std::shared_ptr<CommandManager> &command_manager_;
    };

    class BuiltinCommand_Manage final : public PublicCommand
    {
    public:
        BuiltinCommand_Manage( std::shared_ptr<CommandManager> &manager )
            :   PublicCommand("manage"),
                manager_(manager)
        {}

        ~BuiltinCommand_Manage() = default;

        eExecuteResult process( const CommandInfo &info ) override
        {
            std::string msg = u8"现已启用的命令有 " + std::to_string(manager_->size()) + "\n";
            cq::api::send_group_msg(info.get_group_id(), 
                msg
                    );
            return eExecuteResult::SUCCESS;
        }
    private:
        std::shared_ptr<CommandManager> manager_;
    };

    class BuiltinCommand_Support final : public PublicCommand
    {
    public:
        BuiltinCommand_Support()
            :   PublicCommand("support")
        {
            configure( {"帮咱写"}, "格式 support(帮咱写) GitHub");
        }

        ~BuiltinCommand_Support() = default;

        eExecuteResult process( const CommandInfo &info ) override
        {
            info.send_back("GitHub: https://github.com/brfish/quinbot");
            return eExecuteResult::SUCCESS;
        }

    private:

    };

    static void register_all_builtin_commands( std::shared_ptr<CommandManager> &manager )
    {
        manager->register_command<BuiltinCommand_Test>();
        manager->register_command<BuiltinCommand_Help>(manager);
        manager->register_command<BuiltinCommand_Manage>(manager);
        manager->register_command<BuiltinCommand_Support>();
    }
}
}
}