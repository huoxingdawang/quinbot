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
                    cq::api::send_private_msg(info.get_user_id(), "Hello, private");
                    break;
                case eMessageType::GROUP:
                    cq::api::send_group_msg(info.get_group_id(), "Hello, group");
                    break;
                case eMessageType::DISCUSS:
                    cq::api::send_discuss_msg(info.get_discuss_id(), "Hello, discuss");
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
                "格式: help [可选 待查命令]\n"
                "待查命令为空时列出所有可查命令"
            );
        }

        eExecuteResult process( const CommandInfo &info ) override
        {
            const CommandLine &cl = info.command_line;
            if (cl.size() == 0)
            {
                std::string help;
                for (const auto &str : command_manager_->get_all_help_queryable_commands())
                    help += str + '\n';
                help.pop_back();
                info.send_back(help);
                return eExecuteResult::SUCCESS;
            }

            std::string help;
            if (command_manager_->contains(cl[0]))
                help = command_manager_->get(cl[0])->help_message();
            else
            {
                std::string cname = command_manager_->alias_contains(cl[0]);
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

    class BuiltinCommand_Stop : PrivateCommand
    {
        BuiltinCommand_Stop()
            :   PrivateCommand("stop")
        {}

        eExecuteResult process( const CommandInfo &info ) override
        {
            static size_t index_group_id = 1;
            int64_t to_stop_group_id;
            if (info.command_line.try_as_int64(index_group_id, to_stop_group_id))
            {
                cq::api::send_group_msg(to_stop_group_id, "It will be stopped");
                return eExecuteResult::SUCCESS;
            }
            else
            {
                cq::api::send_private_msg(info.user_id, "The group id is wrong");
                return eExecuteResult::USER_ERROR;
            }

        }

        ~BuiltinCommand_Stop() = default;
        
    };

    static void register_all_builtin_commands( std::shared_ptr<CommandManager> &manager )
    {
        manager->register_command<BuiltinCommand_Test>();
        manager->register_command<BuiltinCommand_Help>(manager);
        manager->register_command<BuiltinCommand_Manage>(manager);
    }
}
}
}