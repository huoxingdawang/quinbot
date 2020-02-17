#pragma once

#include <regex>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <random>

#include <sqlite3.h>

#include "../../plugin.h"
#include "../../bot.h"

namespace dialog
{
    inline std::string wash_str( const std::string &str )
    {
        if (str.size() > 2 && str.front() == '\"' && str.back() == '\"')
            return std::string(str, 1, str.size() - 2);
        return str;
    }

    inline std::string escape( const std::string &str )
    {
        return std::regex_replace(str, std::regex("~"), "&#126;");
    }

    inline std::string unescape( const std::string &str )
    {
        return std::regex_replace(str, std::regex("&#126;"), "~");
    }
}

namespace quinbot
{
namespace plugin
{
    class TeachCommand : public command::GroupCommand
    {
        using eExecuteResult = command::eExecuteResult;
    public:
        TeachCommand( util::Database &dialog_db )
            :   GroupCommand("teach"),
                dialog_db_(dialog_db)
        {
            configure(
                {"教", "学习"},
                "格式 (teach/教/学习) [Option use_regex/使用正则] [Option no_reply/no-reply/不要响应] [Keyword(必须) trigger/触发语句] [Keyword(必须) response/响应语句] [VA(可选) 响应语句...]\n"
                "当触发语句不存在时将学会，存在则覆盖，覆盖时不指定正则将取消其正则识别"
            );
            srand((unsigned int)time(NULL)), rand();
        }

        ~TeachCommand() = default;

        eExecuteResult process( const command::CommandInfo &info ) override
        {
            std::string group_table_name = "G" + std::to_string(info.get_group_id());
            const auto &cl = info.command_line;

            command::ArgsMap args = command::arg_parse(info.command_line, 
            {
                {"trigger", {"触发语句"}},
                {"response", {"响应语句"}},
                command::VARY_ARGS
            },
            {
                {"no_reply", {"no-reply", "不要响应"}},
                {"use_regex", {"使用正则"}},
                {"add", {"增添", "增加", "追加"}}
            });

            if (args.is_bad())
            {
                info.send_back(args.error_message);
                return eExecuteResult::USER_ERROR;
            }

            bool no_reply = args.get_option("no_reply");
            bool use_regex = args.get_option("use_regex");
            bool add = args.get_option("add");
            std::string trigger;
            std::vector<std::string> responses;
            try
            {
                trigger = dialog::wash_str(args.get<std::string>("trigger", "", true));
                responses.push_back(dialog::wash_str(args.get<std::string>("response", "", true)));
                for (size_t i = 0; i < args.va_size(); ++i)
                    responses.push_back(dialog::wash_str(args.get_va<std::string>(i, "")));
            }
            catch ( const std::underflow_error & )
            {
                info.send_back("有效参数过少");
                return eExecuteResult::USER_ERROR;
            }
            catch ( const std::invalid_argument & )
            {
                info.send_back("错误的参数解析");
                return eExecuteResult::USER_ERROR;
            }

            if (!util::starts_with_multi(trigger, quinbot::bot.get_config().command_prefixes).empty())
            {
                info.send_back("[CQ:face,id=146]teach命令不允许使用带有命令前缀的触发语句");
                return eExecuteResult::USER_ERROR;
            }
            std::string response = make_string_group_(responses);
            bool insert_result = insert_teach_(group_table_name, trigger, response, use_regex);
            if (insert_result)
            {
                if (!no_reply)
                {
                    cq::message::Message msg;
                    msg += cq::message::MessageSegment::text("👴学会🌶！");
                    msg += cq::message::MessageSegment::image("B6DCB86562445738385FE3051960670B.jpg");
                    info.send_back(std::to_string(msg));
                }
                cq::logging::debug("teach insert完成", trigger + ": " + response);
            }
            else
            {
                if (!no_reply)
                    info.send_back("👴这就改口");
                update_teach_(group_table_name, trigger, response, use_regex);
            }
            return eExecuteResult::SUCCESS;
        }

    private:
        util::Database &dialog_db_;

        inline std::string make_string_group_( const std::vector<std::string> &strs ) const
        {
            std::string ret;
            static const std::string delimiter = "~";
            for (size_t i = 0; i < strs.size() - 1; ++i)
                ret.append(dialog::escape(strs[i])), ret.append(delimiter);
            ret.append(dialog::escape(strs.back()));
            return ret;
        }

        inline bool insert_teach_( const std::string &table_name, const std::string &key, const std::string &value, bool use_regex = false ) const
        {
            return dialog_db_.insert(table_name, "TRIGGER, RESPONSE, IS_REGEX", "?, ?, ?", [&]( sqlite3_stmt *stmt ) -> bool {
                int32_t result = 0;
                sqlite3_bind_text(stmt, 1, cq::message::unescape(key).c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 2, value.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_int(stmt, 3, use_regex ? 1 : 0);
                return true;
            });
        }

        inline bool update_teach_( const std::string &table_name, const std::string &key, const std::string &value, bool use_regex = false)
        {
            return dialog_db_.update(table_name, {{"RESPONSE", "'" + dialog::wash_str(value) + "'"}, {"IS_REGEX", use_regex ? "1" : "0"}}, "TRIGGER = '" + dialog::wash_str(cq::message::unescape(key)) + "'");
        }
    };

    class ForgetCommand final : public command::GroupCommand
    {
        using eExecuteResult = command::eExecuteResult;
    public:
        ForgetCommand( util::Database &dialog_db )
            :   command::GroupCommand("forget"),
                dialog_db_(dialog_db)
        {
            configure(
                {"遗忘", "忘记"}, 
                "格式 (forget/遗忘/忘记) [响应语句]");
        }

        ~ForgetCommand() = default;

        eExecuteResult process( const command::CommandInfo &info ) override
        {
            std::string group_table_name = "G" + std::to_string(info.get_group_id());
            const auto &cl = info.command_line;
            if (cl.size() < 1)
            {
                info.send_back("参数过少 命令格式 forget [触发语句]");
                return eExecuteResult::TOO_FEW_PARAMETER;
            }
            if (cl.size() > 1)
            {
                info.send_back("参数过多，如要忽略空格，使用\"\"标记");
                return eExecuteResult::TOO_MANY_PARAMETER;
            }
            std::string key = dialog::wash_str(cl[0]);
            bool delete_result = delete_teach_(group_table_name, key);
            if (delete_result)
            {
                info.send_back("?👴不记得了");
            }
            else
            {
                info.send_back("👴都不晓得这句话，遗忘个🐔儿");
            }
            return eExecuteResult::SUCCESS;
        }

    private:
        util::Database &dialog_db_;

        inline bool delete_teach_( const std::string &table_name, const std::string &key )
        {
            return dialog_db_.remove(table_name, "TRIGGER = '" + dialog::wash_str(key) + "'");
        }
    };

    class DialogPlugin : public Plugin
    {
    public:
        DialogPlugin()
            :   Plugin("dialog")
        {}

        void on_enable() override
        {
            if (dialog_db_.open(cq::dir::app() + "/dialog.db"))
                bot.get_logger().info("SQLite", "数据库dialog打开成功");
            else
                bot.get_logger().error("SQLite", "数据库dialog打开失败");
            bot.get_command_manager()->register_command<TeachCommand>(dialog_db_);
            bot.get_command_manager()->register_command<ForgetCommand>(dialog_db_);
        }

        void on_disable() override
        {
            dialog_db_.close();
        }

        void on_group_msg( const cq::event::GroupMessageEvent &e ) override
        {
            if(e.raw_message == "唯一指定邮箱")
            {
                cq::api::send_group_msg(e.group_id, "1748065414@qq.com[CQ:image,file=5C3C2FDF2E8861AE5E0886DAD84F4A19.jpg]");
                return;
            }

            if (!util::starts_with_multi(e.raw_message, quinbot::bot.get_config().command_prefixes).empty())
                return;
            std::string group_table_name = "G" + std::to_string(e.group_id);
            std::string content = get_if_exists_(group_table_name, e.raw_message);
            
            if (!content.empty())
            {
                cq::api::send_group_msg(e.group_id, content);
                return;
            }
            content = match_regex_(group_table_name, e.raw_message);
            if (!content.empty())
            {
                cq::api::send_group_msg(e.group_id, content);
                return;
            }

        }
    
    private:
        util::Database dialog_db_;

        std::string select_one_response_( const std::string &strs ) const
        {
            std::vector<std::string> split_result;
            command::inner_split(strs, split_result, "~", "\"", "\"");
            return *util::select_randomly(split_result.begin(), split_result.end());
        }

        bool is_regex_can_match_( const std::string &str, const std::regex &re )
        {
            std::vector<std::string> split_result;
            command::inner_split(str, split_result, "", "\"", "\"");
            if (split_result.size() == 1)
                return std::regex_search(str, re);
            /*for (auto &s : split_result)
            {
                if (util::starts_with(s, "[CQ:") && !util::starts_with(s, "[CQ:fa") && !util::starts_with("[CQ:at"))
                {
                    if (std::regex_match)
                }
            }*/
        }

        std::string match_regex_( const std::string &table_name, const std::string &key )
        {
            char *err_msg = nullptr;
            std::vector<std::string> contents;
            contents.push_back(key);

            dialog_db_.select(table_name, "TRIGGER,RESPONSE", "IS_REGEX = 1", []( void *data, int row_count, char **rows, char **cols ) ->int {
                std::vector<std::string> &result = *reinterpret_cast<std::vector<std::string> *>(data);
                std::string trigger_str, response_str;
                std::string row, col;
                for (int i = 0; i < row_count; ++i)
                {
                    row = rows[i];
                    col = cols[i];
                    if (col == "TRIGGER") { trigger_str = row; }
                    if (col == "RESPONSE") { response_str = row; }
                }
                std::regex re(trigger_str);
                try
                {
                    if (std::regex_search(result[0], re))
                    {
                        std::vector<std::string> response_strs;
                        command::inner_split(response_str, response_strs, "~", "\"", "\"");
                        for (auto &str : response_strs)
                            result.push_back(dialog::unescape(str));
                    }
                }
                catch(...)
                {

                }
                return 0;
            }, &contents, err_msg);
            if (contents.size() > 1)
                return select_one_response_(*util::select_randomly(contents.begin() + 1, contents.end()));
            return "";
        }

        std::string get_if_exists_( const std::string &table_name, const std::string &key )
        {
            std::string content;
            char *err_msg = nullptr;
            dialog_db_.select(table_name, "TRIGGER = '" + key + "'", []( void *data, int row_count, char **rows, char **cols ) ->int {
                for (int i = 0; i < row_count; ++i)
                {
                    std::string row = rows[i];
                    std::string col = cols[i];
                    if (col == "RESPONSE")
                    {
                        *reinterpret_cast<std::string *>(data) = row;
                        break;
                    }
                }
                return 0;
            }, &content, err_msg);
            if (!content.empty())
                return select_one_response_(content);
            return content;
        }

    };
}
}