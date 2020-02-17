#ifndef __QUINBOT_COMMAND_PROCESSER_H__
#define __QUINBOT_COMMAND_PROCESSER_H__

#include <memory>

#include "../../cqsdk/event.h"

#include "command.h"
#include "command_manager.h"
#include "command_parser.h"

namespace quinbot
{
namespace command
{
    class CommandProcesser
    {
    public:
        explicit CommandProcesser( const std::shared_ptr<CommandManager> &manager, const std::shared_ptr<CommandParser> &parser )
            :   manager_(manager),
                parser_(parser)
        {}

        template<typename MessageEventType>
        eExecuteResult run( const MessageEventType &event )
        {
            CommandLine cl;
            eParseResult parse_result = parser_->parse(manager_, event.message, cl);
            eExecuteResult result = eExecuteResult::UNKNOWN;
            if (parse_result == eParseResult::SUCCESS)
            {
                result = excute_command_(cl, event);
                return result;
            }
            return result;
        }

    private:
        std::shared_ptr<CommandManager> manager_;
        std::shared_ptr<CommandParser> parser_;

        inline eExecuteResult excute_command_( const CommandLine &cl, const cq::PrivateMessageEvent &e ) const
        {
            auto ptr = manager_->get(cl.name(), eCommandType::PRIVATE);
            if (ptr)
                return ptr->process(CommandInfo(e.user_id, cl));
            return manager_->get(cl.name(), eCommandType::PUBLIC)->process(CommandInfo(e.user_id, cl));
        }

        inline eExecuteResult excute_command_( const CommandLine &cl, const cq::GroupMessageEvent &e ) const
        {
            auto ptr = manager_->get(cl.name(), eCommandType::GROUP);
            if (ptr)
                return ptr->process(CommandInfo(e.user_id, e.group_id, cl));
            return manager_->get(cl.name(), eCommandType::PUBLIC)->process(CommandInfo(e.user_id, e.group_id, cl));
        }

        inline eExecuteResult excute_command_( const CommandLine &cl, const cq::DiscussMessageEvent &e ) const
        {
            auto ptr = manager_->get(cl.name(), eCommandType::DISCUSS);
            if (ptr)
                return ptr->process(CommandInfo(e.user_id, e.discuss_id, cl, false));
            return manager_->get(cl.name(), eCommandType::PUBLIC)->process(CommandInfo(e.user_id, e.discuss_id, cl, false));
        }
    };
}
}

#endif