#pragma once

#include "../../cqsdk/dir.h"

#include "../bot.h"
#include "../command/command.h"
#include "../enums.h"
#include "../plugin.h"
#include "../utils/file_utils.h"
#include "../bot.h"
#include "../command/arg_parser.h"

namespace quinbot::test
{
    class TestCommand : public command::PublicCommand
    {
    public:
        TestCommand()
            :   command::PublicCommand("justtest")
        {
            set_aliases({"just"});
        }

        command::eExecuteResult process( const command::CommandInfo &info ) override
        {
            auto &logger = bot.get_logger();
            auto &cl = info.command_line;
            cq::message::Message msg;
            //msg += cq::message::MessageSegment::at(1748065414);
            info.send_back("[CQ:at,qq=1748065414]");
            /*command::ArgsMap args = command::arg_parse(cl, {
                {"a1", {"A1", "AA"}},
                {"b1", {"B1", "BB"}},
                quinbot::command::VARY_ARGS
            }, {});
            if (args.is_bad())
            {
                cq::logging::debug("bad_error", "?");
                info.send_back(args.error_message.empty() ? "ERROR" : args.error_message);
                return command::eExecuteResult::USER_ERROR;
            }
            for (size_t i = 0; i < args.va_size(); ++i)
                info.send_back("msg=" + args.get_va<std::string>(i, ""));*/
            return command::eExecuteResult::SUCCESS;
        }

        ~TestCommand()
        {}
    };

    class SaveImageCommand final: public command::PublicCommand
    {
    public:
        SaveImageCommand()
            :   command::PublicCommand("save")
        {}

        ~SaveImageCommand() = default;

        command::eExecuteResult process( const command::CommandInfo &info ) override
        {
            static int idx = 1;
            const command::CommandLine &cl = info.command_line;
            for (size_t i = 0; i < cl.size(); ++i)
            {
                std::string out_path = cq::dir::app("images") + std::to_string(idx++) + ".jpg";
                auto img = cl.as_image(i);
                bot.get_logger().debug("url", img.url());
                util::write_binary(img.data(), out_path);
            }
            
            return command::eExecuteResult::SUCCESS;
        }
    };

    class EchoCommand final : public command::PublicCommand
    {
    public:
        EchoCommand()
            :   PublicCommand("echo")
        {}

        ~EchoCommand() = default;

        command::eExecuteResult process( const command::CommandInfo &info ) override
        {
            std::string echo_msg = "Echo!: ";
            auto &cl = info.command_line;
            for (size_t i = 0; i < cl.size(); ++i)
            {
                echo_msg += cl[i];
                if (i != cl.size() - 1)
                    echo_msg += ',';
            }
            info.send_back(echo_msg);
            return command::eExecuteResult::SUCCESS;
        }

    private:

    };
}

namespace quinbot::test
{
    class TestPlugin final : public plugin::Plugin
    {
    public:
        TestPlugin()
            :   Plugin("test")
        {
        }

        void on_enable() override
        {
            auto &manager = bot.get_command_manager();
            manager->register_command<TestCommand>();
            manager->register_command<SaveImageCommand>();
            manager->register_command<EchoCommand>();
        }

        void on_group_msg( const cq::event::GroupMessageEvent &e ) override
        {
            std::string msg = e.message;
            if (msg == u8"翟狗biss" || msg == u8"翟🐶biss")
                cq::api::send_group_msg(e.group_id, "确实");
            e.block();
        }

    private:

    };
}