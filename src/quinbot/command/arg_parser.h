#pragma once

#include <initializer_list>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <algorithm>

#include "arg_map.h"
#include "command_line.h"
#include "inner_spliter.h"

namespace quinbot
{
namespace command
{
    static const std::string err_too_many_args = "参数过多";
    static const std::string err_too_few_args = "参数过少";
    static const std::string err_unknown_option = "未知的Option ";
    static const std::string err_already_used_option = "Option已经被声明或重复";
    static const std::string err_position_arg_parse_after_keyword_arg = "Keyword参数应当出现在最后";
    static const std::string err_keyword_arg_parse = "Keyword参数解析失败";
    static const std::string err_unknown_keyword = "未知的Keyword ";
    static const std::string err_already_used_keyword = "Keyword已经被声明或重复";

    static const std::pair<std::string, std::vector<std::string>> VARY_ARGS = {"__VA_FLAG__", {}};

    ArgsMap arg_parse_all_option_( const CommandLine &cl )
    {
        auto it = cl.begin();
        auto end = cl.end();
        ArgsMap result;
        for (; it != end; ++it)
            result.add_option(*it);
        return result;
    }

    ArgsMap arg_parse_( const CommandLine &cl )
    {
        auto it = cl.begin();
        auto end = cl.end();
        ArgsMap result;
        for (; it != end; ++it)
        {
            std::string prefix = util::starts_with_multi(*it, {"-", "/"});
            if (!prefix.empty())
            {
                result.add_option(std::string(*it, prefix.size()));
                continue;
            }
            std::vector<std::string> split_result;
            inner_split(*it, split_result, "=", "\"", "\"", true);
            if (split_result.size() == 1)
                result.add_option(*it);
            else if (split_result.size() == 2)
                result.add_arg(split_result[0], split_result[1]);
        }
        return result;
    }

    using KeyIndexList = std::vector<std::pair<std::string, std::vector<std::string>>>;

    enum eFindResult { FOUND, NOT_FOUND, ALREADY_UESD };

    eFindResult find_key( std::string &out, const std::string &key, const KeyIndexList &list, KeyIndexList::const_iterator offset, const std::unordered_set<std::string> &flag )
    {
        for (auto it = offset; it != list.end(); ++it)
        {
            auto [main_key, matches] = *it;
            if (main_key == key || std::find(matches.begin(), matches.end(), key) != matches.end())
            {
                if (flag.find(main_key) != flag.end())
                    return ALREADY_UESD;
                out = main_key;
                return FOUND;
            }
        }
        return NOT_FOUND;
    }

    ArgsMap arg_parse_force_keyword( const CommandLine &cl, const KeyIndexList &order, const KeyIndexList &options )
    {
        bool parsing_option_arg = true;
        auto it = cl.begin();
        auto end = cl.end();
        ArgsMap result;
        std::unordered_set<std::string> flag, option_flag;

        auto order_it = order.begin();

        for (; it != end; ++it)
        {
            std::vector<std::string> split_result;
            inner_split(*it, split_result, "=", "\"", "\"", true);
            if (split_result.size() == 1)
            {
                if (parsing_option_arg)
                {
                    std::string prefix = util::starts_with_multi(*it, {"-", "/", "要求", "应当", "需要"});
                    std::string option = prefix.empty() ? *it : std::string(*it, prefix.size());
                    std::string main_key;
                    eFindResult option_result = find_key(main_key, option, options, options.begin(), option_flag);
                    if (option_result == ALREADY_UESD)
                    {
                        result.set_error(err_already_used_option + "(" + option + ")");
                        return result;
                    }
                    else if (option_result == NOT_FOUND)
                    {
                        result.set_error(err_unknown_option + "(" + option + ")");
                        return result;
                    }
                    else
                    {
                        result.add_option(main_key);
                        option_flag.insert(main_key);
                        continue;
                    }
                }
                else
                {
                    result.set_error(err_position_arg_parse_after_keyword_arg);
                    return result;
                }
            }
            else if (split_result.size() == 2)
            {
                parsing_option_arg = false;
                std::string main_key;
                eFindResult key_result = find_key(main_key, split_result[0], order, order.begin(), flag);
                if (key_result == ALREADY_UESD)
                {
                    result.set_error(err_already_used_keyword + "(" + split_result[0] + ")");
                    return result;
                }
                else if (key_result == NOT_FOUND)
                {
                    result.set_error(err_unknown_keyword + "(" + split_result[0] + ")");
                    return result;
                }
                else
                {
                    result.add_arg(main_key, split_result[1]);
                    flag.insert(main_key);
                }
            }
            else
            {
                result.set_error(err_keyword_arg_parse);
                return result;
            }
        }
        return result;
    }

    ArgsMap arg_parse( const CommandLine &cl, const KeyIndexList &order, const KeyIndexList &options )
    {
        bool parsing_option_arg = true;
        bool parsing_keyword_arg = false;
        auto it = cl.begin();
        auto end = cl.end();
        ArgsMap result;
        std::unordered_set<std::string> flag, option_flag;

        auto order_it = order.begin();

        for (; it != end; ++it)
        {
            if (parsing_option_arg)
            {
                std::string prefix = util::starts_with_multi(*it, {"-", "/", "要求", "应当", "需要"});
                if (!prefix.empty())
                {
                    std::string option = std::string(*it, prefix.size());
                    std::string main_key;
                    eFindResult option_result = find_key(main_key, option, options, options.begin(), option_flag);
                    if (option_result == ALREADY_UESD)
                    {
                        result.set_error(err_already_used_option + "(" + option + ")");
                        return result;
                    }
                    else if (option_result == NOT_FOUND)
                    {
                        result.set_error(err_unknown_option + "(" + option + ")");
                        return result;
                    }
                    else
                    {
                        result.add_option(main_key);
                        option_flag.insert(main_key);
                        continue;
                    }
                }
                else
                {
                    parsing_option_arg = false;
                }
            }
            std::vector<std::string> split_result;
            inner_split(*it, split_result, "=", "\"", "\"", true);
            if (split_result.size() == 1)
            {
                if (parsing_keyword_arg)
                {
                    result.set_error(err_position_arg_parse_after_keyword_arg);
                    return result;
                }
                if (order_it == order.end() && (order_it - 1)->first == "__VA_FLAG__")
                {
                    result.add_va_arg(*it);
                    continue;
                }
                if (order_it == order.end())
                {
                    result.set_error(err_too_many_args);
                    return result;
                }
                if ((order_it)->first == "__VA_FLAG__")
                {
                    result.add_va_arg(*it);
                    continue;
                }
                auto [main_key, match_strs] = *order_it;
                flag.insert(main_key);
                ++order_it;
                result.add_arg(main_key, *it);
            }
            else if (split_result.size() == 2)
            {
                parsing_keyword_arg = true;
                std::string main_key;
                eFindResult key_result = find_key(main_key, split_result[0], order, order.begin(), flag);
                if (key_result == ALREADY_UESD)
                {
                    result.set_error(err_already_used_keyword + "(" + split_result[0] + ")");
                    return result;
                }
                else if (key_result == NOT_FOUND)
                {
                    result.set_error(err_unknown_keyword + "(" + split_result[0] + ")");
                    return result;
                }
                else
                {
                    result.add_arg(main_key, split_result[1]);
                    flag.insert(main_key);
                }
                
            }
            else
            {
                result.set_error(err_keyword_arg_parse);
                return result;
            }
        }
        return result;
    }

    ArgsMap arg_parse( const CommandLine &cl, bool all_option = false )
    {
        if (all_option)
            return arg_parse_all_option_(cl);
        return arg_parse_(cl);
    }


    class ArgParser
    {
        using StrVec = std::vector<std::string>;
    public:
        ArgsMap parse( const CommandLine &cl, size_t left, size_t right, bool all_option = false )
        {
            ArgsMap result;
            if (cl.size() == 0)
                return result;
            if (left < 0)   left = 0;
            if (right >= cl.size()) right = cl.size() - 1;
            if (right < 0) right = 0;
            for (size_t index = left; index <= right; ++index)
            {
                auto &cur = cl[index];
                if (all_option)
                {
                    result.add_option(cur);
                    continue;
                }
                std::string option_tag = get_option_tag(cur);
                if (!option_tag.empty())
                {
                    result.add_option(std::string(cur, option_tag.size(), cur.size() - option_tag.size()));
                    continue;
                }

            }
            return result;
        }

        bool is_option( const std::string &arg_str ) const
        {
            for (auto &option_tag : option_tags_)
            {
                if (util::starts_with(arg_str, option_tag))
                    return true;
            }
            return false;
        }

        std::string get_option_tag( const std::string &arg_str ) const
        {
            for (auto &option_tag : option_tags_)
            {
                if (util::starts_with(arg_str, option_tag))
                    return option_tag;
            }
            return "";
        }



    private:
        std::string operator_assign_;
        std::string operator_list_;
        std::vector<std::string> option_tags_;
    };
}
}