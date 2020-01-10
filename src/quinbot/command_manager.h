#ifndef __QUINBOT_COMMAND_MANAGER_H__
#define __QUINBOT_COMMAND_MANAGER_H__

#include <unordered_map>
#include <string>
#include <memory>

#include "command.h"

namespace quinbot
{
    class CommandManager
    {
        using CommandPointer = std::shared_ptr<Command>;
    public:
        CommandManager();

        inline bool contains( const std::string name ) const
        {
            return matcher_.find(name) != matcher_.end();
        }

        inline void add_command( const Command &command )
        {
            matcher_.insert(std::make_pair(command.name(), std::make_shared<Command>(command)));
        }

        inline void remove_command( const std::string &name )
        {
            if (!contains(name))
                return;
            matcher_.erase(name);
        }

        inline const CommandPointer &operator[]( const std::string &key ) const
        {
            return matcher_.at(key);
        }

        inline const CommandPointer &match( const std::string &name ) const
        {
            return matcher_.at(name);
        }

        inline const CommandPointer &match( const CommandLine &cl ) const
        {
            return matcher_.at(cl.name());
        }


    private:
        std::unordered_map<std::string, CommandPointer> matcher_;
    };

}

#endif