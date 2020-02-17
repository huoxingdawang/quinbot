#pragma once

#include <string>
#include <vector>

#include <sqlite3.h>

#include "../../plugin.h"
#include "../../bot.h"

namespace quinbot
{
namespace plugin
{
    bool is_data_exists( sqlite3 *db, const std::string &table_name, const std::string &condition )
    {
        if (!db)
            return false;
        std::string sentence = "SELECT * FROM " + table_name + " WHERE " + condition;
        int32_t count = 0;
        char *err_msg = nullptr;
        sqlite3_exec(db, sentence.c_str(), [](void *data, int row_count, char **rows, char **cols) ->int {
            //if (row_count != 0)
            *reinterpret_cast<int32_t *>(data) = 1;
            return 0;
        }, &count, &err_msg);
        return count == 1;
    }

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
                "格式 (teach/教/学习) [触发语句] [响应语句] [可选 no-reply/闭嘴学/安静学]\n"
                "当触发语句不存在时将学会，存在则覆盖"
            );
        }

        ~TeachCommand() = default;

        eExecuteResult process( const command::CommandInfo &info ) override
        {
            std::string group_table_name = "G" + std::to_string(info.get_group_id());
            const auto &cl = info.command_line;
            bool no_reply_v1 = false, no_reply_v2 = false;
            if (cl.size() < 2)
            {
                info.send_back("参数过少 命令格式 teach [触发语句] [响应语句]");
                return eExecuteResult::TOO_FEW_PARAMETER;
            }
            if (cl.size() > 2)
            {
                if (cl[2] == "no-reply")
                    no_reply_v1 = true;
                else if (cl[2] == "闭嘴学" || cl[2] == "安静学")
                    no_reply_v1 = no_reply_v2 = true;
                else
                {
                    info.send_back("参数过多，如要忽略空格，使用\"\"标记");
                    return eExecuteResult::TOO_MANY_PARAMETER;
                }
            }
            std::string key = wash_str_(cl[0]);
            std::string value = wash_str_(cl[1]);
            if (key == "唯一指定邮箱")
            {
                info.send_back("憋想改");
                return eExecuteResult::USER_ERROR;
            };
            if (!util::starts_with_multi(key, quinbot::bot.get_config().command_prefixes).empty())
            {
                info.send_back("[CQ:face,id=146]teach命令不允许使用带有命令前缀的触发语句");
                return eExecuteResult::USER_ERROR;
            }
            bool insert_result = insert_teach_(group_table_name, key, value);
            if (insert_result)
            {
                if (!no_reply_v1)
                {
                    cq::message::Message msg;
                    msg += cq::message::MessageSegment::text("👴学会🌶！");
                    msg += cq::message::MessageSegment::image("EEB654B348C8A929DD74A3AE912CF007.jpg");
                    info.send_back(std::to_string(msg));
                }
                if (no_reply_v2)
                    info.send_back("(...)脏话");
                cq::logging::debug("teach insert完成", key + ": " + value);
            }
            else
            {
                if (!no_reply_v1)
                    info.send_back("👴这就改口");
                if (no_reply_v2)
                    info.send_back("(...)脏话");
                update_teach_(group_table_name, key, value);
            }
            return eExecuteResult::SUCCESS;
        }

    private:
        //sqlite3 *dialog_db_;
        util::Database &dialog_db_;

        inline std::string wash_str_( const std::string &str ) const
        {
            if (str.size() > 2 && str.front() == '\"' && str.back() == '\"')
                return std::string(str, 1, str.size() - 2);
            return str;
        }

        bool insert_teach_( const std::string &table_name, const std::string &key, const std::string &value ) const
        {
            return dialog_db_.insert(table_name, "TRIGGER, RESPONSE, IS_REGEX", "?, ?, 0", [&]( sqlite3_stmt *stmt ) -> bool {
                int32_t result = 0;
                sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 2, value.c_str(), -1, SQLITE_TRANSIENT);
                return true;
            });
            /*std::string sentence =  "INSERT INTO " + table_name + " (TRIGGER, RESPONSE, IS_REGEX)" \
                                    "VALUES(?, ?, 1)";
            sqlite3_stmt *stmt = nullptr;
            bool insert_result = true;
            int result = sqlite3_prepare_v2(dialog_db_, sentence.c_str(), -1, &stmt, nullptr);
            if (result == SQLITE_OK)
            {
                sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 2, value.c_str(), -1, SQLITE_TRANSIENT);

                int step_result = sqlite3_step(stmt);
                if (step_result == SQLITE_DONE)
                    insert_result = true;
                else
                    insert_result = false;
            }
            else
                insert_result = false;
            sqlite3_finalize(stmt);
            return insert_result;*/
        }

        bool update_teach_( const std::string &table_name, const std::string &key, const std::string &value)
        {
            return dialog_db_.update(table_name, {{"RESPONSE", "'" + value + "'"}}, "TRIGGER = '" + key + "'");
            /*std::string sentence = "UPDATE " + table_name + " SET RESPONSE = '" + value + "' WHERE TRIGGER = '" + key + "'";
            sqlite3_stmt *stmt = nullptr;
            bool update_result = true;
            int result = sqlite3_prepare_v2(dialog_db_, sentence.c_str(), -1, &stmt, nullptr);
            if (result == SQLITE_OK)
            {
                int step_result = sqlite3_step(stmt);
                if (step_result == SQLITE_DONE)
                    update_result = true;
                else
                    update_result = false;
            }
            else
                update_result = false;
            sqlite3_finalize(stmt);
            return update_result;*/
        }

        /*void create_table_( int64_t group_id )
        {
            std::string sql_sentence =   "CREATE TABLE GROUP" + std::to_string(group_id) + "(\n"
                                "ID INT PRIMARY KEY NOT NULL,\n"
                                "TRIGGER TEXT NOT NULL,\n"
                                "RESPONSE TEXT NOT NULL);";
            sqlite3_stmt *stmt = NULL;
            int32_t result = sqlite3_prepare_v2(db_, sql_sentence.c_str(), -1, &stmt, NULL);
            if (result == SQLITE_OK)
            {
                int32_t step_result = sqlite3_step(stmt);
                if (step_result == SQLITE_OK)
                    bot.get_logger().info("数据库", "群" + std::to_string(group_id) + " 对话表创建成功");
                else
                    bot.get_logger().error("数据库", "群" + std::to_string(group_id) + " 对话表创建失败");
            }
            else
                bot.get_logger().error("数据库", "群" + std::to_string(group_id) + " 对话表创建失败");
            sqlite3_finalize(stmt);
        }*/
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
            std::string key = wash_str_(cl[0]);
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
        //sqlite3 *dialog_db_;
        util::Database &dialog_db_;

        inline std::string wash_str_( const std::string &str ) const
        {
            if (str.size() > 2 && str.front() == '\"' && str.back() == '\"')
                return std::string(str, 1, str.size() - 2);
            return str;
        }

        bool delete_teach_( const std::string &table_name, const std::string &key )
        {
            return dialog_db_.remove(table_name, "TRIGGER = '" + key + "'");
            /*std::string sentence = "DELETE FROM " + table_name + " WHERE TRIGGER = '" + key + "'";
            sqlite3_stmt *stmt = nullptr;
            bool delete_result = true;
            int result = sqlite3_prepare_v2(dialog_db_, sentence.c_str(), -1, &stmt, nullptr);
            if (result == SQLITE_OK)
            {
                int step_result = sqlite3_step(stmt);
                if (step_result == SQLITE_DONE)
                    delete_result = true;
                else
                    delete_result = false;
            }
            else
                delete_result = false;
            sqlite3_finalize(stmt);
            return delete_result;*/
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
            //open_dialog_db_();
            dialog_db_.open(cq::dir::app() + "/dialog.db");
            bot.get_command_manager()->register_command<TeachCommand>(dialog_db_);
            bot.get_command_manager()->register_command<ForgetCommand>(dialog_db_);
        }

        void on_disable() override
        {
            dialog_db_.close();
            //close_dialog_db_();
        }

        void on_group_msg( const cq::event::GroupMessageEvent &e ) override
        {
            if(e.raw_message == "唯一指定邮箱")
                cq::api::send_group_msg(e.group_id, "1748065414@qq.com");
            std::string group_table_name = "G" + std::to_string(e.group_id);
            std::string content = get_if_exists_(group_table_name, e.raw_message);
            if (!content.empty())
                cq::api::send_group_msg(e.group_id, content);
        }
    
    private:
        //sqlite3 *dialog_db_ = nullptr;
        util::Database dialog_db_;

        /*inline bool open_dialog_db_()
        {
            if (dialog_db_)
                close_dialog_db_();
            std::string path = cq::dir::app() + "/dialog.db";
            int result = sqlite3_open_v2(path.c_str(), &dialog_db_, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_SHAREDCACHE, NULL);
            if (result == SQLITE_OK)
            {
                quinbot::bot.get_logger().info("SQLite", "打开dialog数据库成功");
                return true;
            }
            quinbot::bot.get_logger().error("SQLite", "打开dialog数据库失败");
            return false;
        }

        inline bool close_dialog_db_()
        {
            if (dialog_db_) 
            {
                sqlite3_close_v2(dialog_db_);
                dialog_db_ = nullptr;
                return true;
            }
            return false;
        }*/

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
            return content;
        }

    };
}
}