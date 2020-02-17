#ifndef __QUINBOT_COMMAND_PARSER_H__
#define __QUINBOT_COMMAND_PARSER_H__

#include <memory>
#include <string>
#include <vector>
#include <unordered_set>

#include "bot.h"
#include "command.h"
#include "command_line.h"
#include "../utils/string_utils.h"
#include "inner_spliter.h"

namespace quinbot
{
namespace command
{
    class CommandParser
    {
    public:
        CommandParser()
            :   spliter_(eCommandSpliter::INNER),
                delimiter_(" "),
                range_left_("\""),
                range_right_("\"")
        {}

        CommandParser( const std::string &split_regex_str )
            :   split_regex_str_(split_regex_str),
                spliter_(eCommandSpliter::CUSTOM)
        {}

        CommandParser( const std::string &delimiter, const std::string &lrange, const std::string &rrange = "" )
            :   delimiter_(delimiter),
                range_left_(lrange),
                spliter_(eCommandSpliter::INNER)
        {
            if (rrange.empty())
                range_right_ = range_left_;
            else
                range_right_ = rrange;
        }

        inline void set_command_spliter( const eCommandSpliter spliter = eCommandSpliter::INNER )
        {
            spliter_ = spliter;
        }

        inline void set_split_regex_str( const std::string &regex_str )
        {
            split_regex_str_ = regex_str;
        }

        inline void set_arg_range( const std::string &left , const std::string &right )
        {
            range_left_ = left;
            range_right_ = right;
        }

        inline void set_arg_range( const std::string &str )
        {
            set_arg_range(str, str);
        }

        inline void set_stripped( bool stripped )
        {
            stripped_ = stripped;
        }

        inline void set_strip_chars( const std::string &chars )
        {
            strip_chars_ = chars;
        }

        eParseResult parse( std::shared_ptr<CommandManager> manager, const std::string &str, CommandLine &out )
        {
            std::string raw_str = str;
            strip_(raw_str);
            std::string prefix = manager->has_command_prefix(raw_str);
            if (!prefix.empty())
            {
                split_(raw_str, workplace_);
                std::string command_name = get_command_name_(prefix);
                if (manager->contains(command_name))
                {
                    cq::logging::debug(u8"执行命令", "已查找到命令 [" + command_name + u8"]");
                    parse_to_command_line_(command_name, out);
                }
                else
                {
                    std::string original_name = manager->alias_contains(command_name);
                    if (!original_name.empty())
                    {
                        cq::logging::debug(u8"执行命令", "已查找到命令别名 [" + command_name + u8"] 对应命令 [" + original_name + u8"]");
                        parse_to_command_line_(original_name, out);
                    }
                    else
                        return eParseResult::NOT_A_COMMAND;
                }
            }
            else
            {
                return eParseResult::NOT_A_COMMAND;
            }
            return eParseResult::SUCCESS;
        }

    private:
        static const std::string default_split_regex_str;

        eCommandSpliter spliter_;
        std::string split_regex_str_;

        std::string delimiter_;
        std::string range_left_;
        std::string range_right_;

        bool stripped_;
        std::string strip_chars_;

        mutable std::vector<std::string> workplace_;

        inline std::string get_command_name_( const std::string &prefix ) const
        {
            return std::string(workplace_[0], prefix.size(), workplace_[0].size() - prefix.size());
        }

        inline std::string escape_( const std::string &str ) const
        {
            static std::unordered_set<char> need_escape_ = {
                '\\', '^', '$', '*', '+', '?', '.' , '{', '}', '[', ']', '(', ')', '|'
            };
            std::string escaped_str;
            for (size_t i = 0; str[i]; ++i)
            {
                if (need_escape_.find(str[i]) != need_escape_.end())
                    escaped_str += '\\' + str[i];
                else
                    escaped_str += str[i];
            }
            return escaped_str;
        }

        inline void strip_( std::string &str )
        {
            if (stripped_)
                str = util::strip_str(str, strip_chars_);
        }

        void split_( const std::string &str, std::vector<std::string> &out )
        {
            if (spliter_ == eCommandSpliter::INNER)
            {
                inner_split(str, out, delimiter_, range_left_, range_right_);
            }
            else if (spliter_ == eCommandSpliter::CUSTOM)
            {
                if (split_regex_str_.empty())
                    split_regex_str_ = default_split_regex_str;
                util::split_with_regex(out, str, split_regex_str_);
            }
        }

        void parse_to_command_line_( const std::string &command_name, CommandLine &cl )
        {
            cl.clear();
            cl.set_name(command_name);
            for (auto it = workplace_.begin() + 1; it != workplace_.end(); ++it)
                cl.add_arg(*it);
        }
    };

    const std::string CommandParser::default_split_regex_str = u8R"( \s*(?![^"]*" ))";
}
}

#endif