#ifndef __QUINBOT_CONFIG_H__
#define __QUINBOT_CONFIG_H__

#include <initializer_list>
#include <string>
#include <unordered_map>
#include <vector>

#include "../json/json.hpp"

namespace quinbot
{
    class Config
    {
        using json = nlohmann::json;
    public:
        Config();

        Config( std::string &path );

        Config( const json &json_config );

        Config( const Config &other );

        Config &operator=( const Config &other );

        void set_superuser( int64_t user_id );

        void set_command_start( const std::vector<std::string> &starts );

        void set_command_start( std::initializer_list<std::string> starts );

        void set_command_seperator( const std::vector<std::string> &seperaotrs );

        void set_command_seperator( std::initializer_list<std::string> seperators );

        inline int64_t get_superuser() const
        {
            return superuser_;
        }

        inline const std::vector<std::string> &get_command_start() const
        {
            return command_start_;
        }

        inline const std::vector<std::string> &get_command_seperator() const
        {
            return command_seperator_;
        }
    private:
        int64_t superuser_;
        std::vector<std::string> command_start_;
        std::vector<std::string> command_seperator_;
    };
}

#endif