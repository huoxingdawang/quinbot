#ifndef __QUINBOT_COMMAND_H__
#define __QUINBOT_COMMAND_H__

#include <string>
#include <functional>
#include <optional>

#include "enums.h"
#include "command_line.h"

namespace quinbot
{
    class Command
    {
    public:
        Command( const std::string &name )
            :   name_(name)
        {
        }

        Command( const std::string &name, std::function<eExcuteResult ( const CommandLine & )> process_function)
            :   name_(name),
                process_function_(process_function)
        {
        }

        virtual ~Command() = 0;

        virtual eExcuteResult process( const CommandLine &cl ) = 0;

        inline eExcuteResult excute( const CommandLine &cl )
        {
            if (process_function_.has_value())
                return (*process_function_)(cl);
            return process(cl);
        }

        inline std::string name() const
        {
            return name_;
        }

        inline eExcuteResult operator()( const CommandLine &cl )
        {
            return excute(cl);
        }
    
    protected:
        std::string name_;
        std::optional<std::function<eExcuteResult ( const CommandLine & )>> process_function_;
    };

    class TestCommand : public Command
    {
        TestCommand()
            :   Command("Test")
        {}

        eExcuteResult process( const CommandLine &cl ) override
        {
            printf("ahah");
        }
    };
}

#endif