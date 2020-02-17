#pragma once

#include <initializer_list>
#include <string>
#include <unordered_map>
#include <vector>
#include <fstream>

#include "utils/json.hpp"

#include "enums.h"

namespace quinbot
{
    struct Config
    {
        using json = nlohmann::json;
        using StringArray = std::vector<std::string>;
        using eCommandSpliter = command::eCommandSpliter;

        bool developing;
        StringArray command_prefixes;
        eCommandSpliter command_spliter;
        std::string command_split_regex;
        std::string command_delimiter;
        StringArray command_multi_delimiters;
        std::string command_arg_range_left;
        std::string command_arg_range_right;
        bool case_sensitive;  
        bool stripped;
        std::string strip_chars;
        bool allow_aliases;
        bool allow_prefix_command;

        bool enable_logger;
        eLoggerType logger_type;
        std::string logger_output_file_path;

        std::string nickname;
        int64_t superuser;
        std::vector<int64_t> users;
        std::vector<int64_t> groups;
        std::vector<int64_t> discussion_groups;

        Config() = default;

        Config( const Config &other ) = default;

        Config &operator=( const Config &other ) = default;

        Config &operator=( const json &json_config )
        {
            from_json(json_config);
            return *this;
        }

        Config( const json &json_config )
        {
            from_json(json_config);
        }

        Config &from_json( const json &j )
        {
            developing = j.at("developing").get<bool>();
            command_prefixes = j.at("command_prefixes").get<StringArray>();
            command_spliter = j.at("command_spliter").get<eCommandSpliter>();
            command_split_regex = j.at("command_split_regex").get<std::string>();
            command_delimiter = j.at("command_delimiter").get<std::string>();
            command_multi_delimiters = j.at("command_multi_delimiters").get<StringArray>();
            command_arg_range_left = j.at("command_arg_range_left").get<std::string>();
            command_arg_range_right = j.at("command_arg_range_right").get<std::string>();
            case_sensitive = j.at("case_sensitive").get<bool>();
            stripped = j.at("stripped").get<bool>();
            strip_chars = j.at("strip_chars").get<std::string>();
            allow_aliases = j.at("allow_aliases").get<bool>();
            allow_prefix_command = j.at("allow_prefix_command").get<bool>();

            enable_logger = j.at("enable_logger").get<bool>();
            logger_type = j.at("logger_type").get<eLoggerType>();
            logger_output_file_path = j.at("logger_output_file_path").get<std::string>();

            nickname = j.at("nickname").get<std::string>();
            superuser = j.at("superuser").get<int64_t>();
            users = j.at("users").get<std::vector<int64_t> >();
            groups = j.at("groups").get<std::vector<int64_t> >();
            discussion_groups = j.at("discussion_groups").get<std::vector<int64_t> >();

            return *this;
        }

        json to_json()
        {
            json j = {
                {"developing", developing},
                {"command_prefixes", command_prefixes},
                {"command_spliter", command_spliter == eCommandSpliter::INNER ? "inner" : "custom"},
                {"command_split_regex", command_split_regex},
                {"command_delimiter", command_delimiter},
                {"command_multi_delimiters", command_multi_delimiters},
                {"command_arg_range_left", command_arg_range_left},
                {"command_arg_range_right", command_arg_range_right},
                {"case_sensitive", case_sensitive},
                {"stripped", stripped},
                {"strip_chars", strip_chars},
                {"allow_aliases", allow_aliases},
                {"allow_prefix_command", allow_prefix_command},

                {"enable_logger", enable_logger},
                {"logger_type", logger_type},
                {"logger_output_file_path", logger_output_file_path},
                
                {"nickname", nickname},
                {"users", users},
                {"groups", groups},
                {"discussion_groups", discussion_groups},
            };
            return j;
        }
    };
}
