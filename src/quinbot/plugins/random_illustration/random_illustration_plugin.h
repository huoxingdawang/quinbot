#pragma once

#include <chrono>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <queue>
#include <Windows.h>

#include <cpr/cpr.h>
#include <Python.h>
#include <sqlite3.h>

#include "../../plugin.h"
#include "../../bot.h"

#define TIMER_START( ID ) std::chrono::steady_clock::time_point time_start_##ID## = std::chrono::steady_clock::now();
#define TIMER_END( Message, ID ) \
std::chrono::steady_clock::time_point time_end_##ID## = std::chrono::steady_clock::now(); \
std::chrono::duration<double> used_##ID## = time_end_##ID## - time_start_##ID##; \
bot.get_logger().debug(Message, std::to_string(used_##ID##.count()) + " s");

bool sqlite_index_size( sqlite3 *db, int32_t &result_out, const std::string &table_name )
{
    int32_t count = 0;
    char *err_msg = nullptr;
    std::string sentence = "SELECT COUNT(*) FROM " + table_name;
    if (!db)
    {
        return false;
    }
    int exec_result = sqlite3_exec(db, sentence.c_str(), [](void *data, int row_count, char **rows, char **cols) ->int {
        if (row_count == 1 && rows)
        {
            *static_cast<int32_t *>(data) = std::atoi(rows[0]);
            return 0;
        }
        return 1;
    }, &count, &err_msg);
    if (err_msg)
        cq::logging::debug("error_msg", std::string(err_msg));
    if (exec_result == SQLITE_OK)
    {
        result_out = count;
        return true;
    }
    return false;
}

namespace quinbot
{
namespace plugin
{
    class DonateCommand final : public command::PublicCommand
    {
        using eExecuteResult = command::eExecuteResult;
        using CommandInfo = command::CommandInfo;

        struct GroupIllustrationInfo
        {
            std::string from;
            int64_t user_id;
            std::string add_time;
            std::string local_file_name;
        };
    public:
        DonateCommand( util::Database &index_db )
            :   PublicCommand("donate"),
                index_db_(index_db)
        {
            configure(
                {"捐图", "添加涩图"},
                "格式 (donate/捐图/添加/涩图) [涩图1] [涩图2] [涩图3] ...(单次最多10张)"
            );
        }

        eExecuteResult process( const CommandInfo &info ) override
        {
            auto &cl = info.command_line;
            size_t count = 0;
            for (size_t i = 0; i < cl.size() && count <= max_size; ++i)
            {
                if (cl.type_of(i) == command::eCommandArgType::IMAGE)
                {
                    ++count;
                    std::string nickname = cq::api::get_group_member_info(info.get_group_id(), info.get_user_id()).nickname;
                    insert_info_(
                        {   nickname,
                            info.get_user_id(),
                            util::get_current_time_str(), 
                            cl.as_image(i).file_name()
                        });
                    
                }
            }
            cq::message::Message msg;
            if (count != 0)
            {
                msg += cq::message::MessageSegment::at(info.get_user_id());
                msg += cq::message::MessageSegment::text("\n宁一共捐了" + std::to_string(count) + "张涩图，感谢宁对涩图事业的发展");
            }
            else
            {
                msg += cq::message::MessageSegment::at(info.get_user_id());
                msg += cq::message::MessageSegment::text("一张没有，丢人");
            }
            info.send_back(std::to_string(msg));
            return eExecuteResult::SUCCESS;
        }
    private:
        util::Database &index_db_;
        static const size_t max_size = 10;

        bool insert_info_( const GroupIllustrationInfo &info ) const
        {
            auto &logger = quinbot::bot.get_logger();
            int32_t id  = 0;
            if (index_db_.size(id, "GROUP_PIC_INDEX"))
            {
                ++id;
                logger.debug("CURRENT_ID", std::to_string(id));
            }
            else
                logger.error("SQLite", "获取GROUP_PIC_INDEX大小失败");
            return index_db_.insert("GROUP_PIC_INDEX", "ID, ADD_BY, ADD_TIME, LOCAL_NAME, USER_ID", "?, ?, ?, ?, ?", [&]( sqlite3_stmt *stmt ) -> bool {
                sqlite3_bind_int(stmt, 1, id);
                sqlite3_bind_text(stmt, 2, info.from.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 3, info.add_time.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 4, info.local_file_name.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_int64(stmt, 5, info.user_id);
                return true;
            });
        }
    };

    class RecoverCommand final : public command::PublicCommand
    {
        using eExecuteResult = command::eExecuteResult;
        using CommandInfo = command::CommandInfo;
    public:
        RecoverCommand( std::map<int64_t, std::queue<int64_t>> &message_queues )
            :   PublicCommand("recover"),
                message_queues_(message_queues)
        {
            set_aliases({"回收", "撤回涩图", "♲"});
            set_help_message("格式 (回收/撤回涩图/♲)");
        }

        eExecuteResult process( const CommandInfo &info ) override
        {
            int32_t count = 0;
            int32_t failed = 0;
            std::queue<int64_t> &message_queue = message_queues_[info.get_group_id()];
            if (message_queue.empty())
            {
                cq::message::Message msg;
                msg += cq::message::MessageSegment::text("没有可以回收的");
                msg += cq::message::MessageSegment::face(146);
                info.send_back(std::to_string(msg));
                return eExecuteResult::SUCCESS;
            }
            while (!message_queue.empty())
            {
                try
                {
                    cq::api::delete_msg(message_queue.front());
                }
                catch(...)
                {
                    ++failed;
                    quinbot::bot.get_logger().warning("撤回消息", "消息撤回失败");
                }
                message_queue.pop();
                ++count;
            }
            std::string msg = "完事🌶！一共撤回了" + std::to_string(count) + "条涩图信息";
            if (failed != 0)
                msg += " 其中有" + std::to_string(failed) + "条撤回失败，很神秘";
            info.send_back(msg);
            return eExecuteResult::SUCCESS;
        }

    private:
        std::map<int64_t, std::queue<int64_t>> &message_queues_;
    };

    class ILoveSexIllustrationCommand final : public command::PublicCommand
    {
        struct IllustrationInfo
        {
            int64_t uid;
            std::string artist;

            int64_t pid;
            std::string title;

            std::string url;
            std::string local_file_name;
        };

        struct GroupIllustrationInfo
        {
            std::string from;
            int64_t user_id;
            std::string add_time;
            std::string local_file_name;
        };

        using eExecuteResult = command::eExecuteResult;
        using CommandInfo = command::CommandInfo;
        using json = nlohmann::json;
    public:
        ILoveSexIllustrationCommand( std::map<int64_t, std::queue<int64_t>> &message_queues, util::Database &index_db, util::Database &group_index_db_, util::Database &local_index_db )
            :   PublicCommand("random"),
                message_queues_(message_queues),
                index_db_(index_db),
                group_index_db_(group_index_db_),
                local_index_db_(local_index_db)
        {
            this->set_aliases({"来点好康的", "来份色图", "来点涩图", "来点色图", "来份涩图", "来点纸片人"});
            this->set_help_message(
                "格式 (random/来点好康的/来点涩图/来点涩图/来份色图/来份涩图/来点纸片人) [Option r18/色一点/涩一点/色一点!/涩一点!] [Option no_image/noimage/no-image/不显示图片] [Option no_detail/nodetail/no-detail/不显示详细/不显示详情] [Option more/多来几张/多整点] [Option(source=group下配置) dozen/来一打] [Keyword(可选) keyword/tag/关键词] [Keyword(可选) source/src/来源] [Keyword(可选 在source=group下配置) user_id/群友ID/群友id]");
        }

        ~ILoveSexIllustrationCommand()
        {
        }

        eExecuteResult process( const CommandInfo &info ) override
        {
            auto &cl = info.command_line;
            auto &logger = quinbot::bot.get_logger();

            command::ArgsMap args = arg_parse_force_keyword(cl, 
            {
                {"keyword", {"tag", "关键词"}},
                {"source", {"src", "来源"}},
                {"score", {"分数"}},
                {"user_id", {"群友ID", "群友id", "用户"}},
            },
            {
                {"r18", {"色一点", "涩一点", "越色越好", "越涩越好", "色一点!", "涩一点!", "越色越好!", "越涩越好!", "色一点！", "涩一点！", "越色越好！", "越涩越好！"}},
                {"more", {"多来几张", "多整点"}},
                {"no_image", {"noimage", "no-image", "不显示图片"}},
                {"no_detail", {"nodetail", "no-detail", "不显示详细", "不显示详情"}},
                {"from_buffer", {"from_buf", "from-buf", "从现有整"}},
                {"dozen", {"来一打"}}
            });

            if (args.is_bad())
            {
                info.send_back(args.error_message);
                return eExecuteResult::USER_ERROR;
            }        

            bool r18, multi, no_image, no_detail;
            bool from_buffer, from_group;
            std::string keyword;
            int64_t group_user_id = 0;
            bool dozen;
            std::string source;
            int32_t score = 0;

            r18 = args.get_option("r18");
            multi = args.get_option("more");
            no_image = args.get_option("no_image");
            no_detail = args.get_option("no_detail");
            keyword = args.get<std::string>("keyword", "-", false);
            group_user_id = args.get<int64_t>("user_id", 0, false);
            from_buffer = args.get_option("from_buffer");
            source = args.get<std::string>("source", "lolicon", false);
            score = args.get<int32_t>("score", 0, false);

            dozen = args.get_option("dozen");

            cq::logging::debug("r18", std::to_string(r18));
            cq::logging::debug("multi", std::to_string(multi));
            cq::logging::debug("no_image", std::to_string(no_image));
            cq::logging::debug("no_detail", std::to_string(no_detail));
            cq::logging::debug("keyword", keyword);
            cq::logging::debug("source", source);
            cq::logging::debug("from_buffer", std::to_string(from_buffer));

            int num = 1;
            if (multi)
                num = util::range_random(2, 4);

            if (dozen)
                num = 15;

            if (no_image && no_detail)
            {
                info.send_back(std::to_string(cq::message::MessageSegment::text("啥都不要你跟👴说你🐴呢")));
                return eExecuteResult::SUCCESS;
            }

            if (source != "group" && source != "local" && source != "lolicon")
            {
                info.send_back("Keyword (source) 可选值[local/lolicon/group]");
                return eExecuteResult::USER_ERROR;
            }

            if (source == "group")
            {
                std::vector<GroupIllustrationInfo> arts;
                random_group_compensate_(num, arts, group_user_id);
                if (arts.size() == 0)
                {
                    info.send_back("这个人一张也没有诶");
                    return eExecuteResult::SUCCESS;
                }
                int64_t msg_id = info.send_back(format_result_group_info_(arts, info.get_user_id(), no_image, no_detail));
                message_queues_[info.get_group_id()].push(msg_id);
                return eExecuteResult::SUCCESS;
            }

            if (source == "local")
            {
                std::vector<std::string> files;
                if (multi)
                    num = util::range_random(3, 5);
                get_local_illustration_(files, num, keyword, score);
                if (files.size() == 0)
                {
                    info.send_back("本地查找不到相关 关键词请使用英文");
                    return eExecuteResult::SUCCESS;
                }
                for (auto file : files)
                {
                    cq::logging::debug("filename", file.substr(file.rfind('/') + 1));
                    crop_image_(file.substr(file.rfind('/') + 1));
                }

                util::MessageBuilder msg;
                for (auto file : files)
                {
                    try 
                    {
                        util::MessageBuilder msg;
                        msg.image(file);
                        int64_t msg_id = info.send_back(std::to_string(msg)); 
                        message_queues_[info.get_group_id()].push(msg_id); 
                    } 
                    catch(...) 
                    {
                        info.send_back("这张太大了👴发不出来");
                    }
                }
                
                return eExecuteResult::SUCCESS;
                
            }

            std::vector<IllustrationInfo> arts;
            if (!get_illustration_info_(arts, r18, num, keyword))
            {
                cq::message::Message msg;
                msg += cq::message::MessageSegment::image("E5E317AA442161A12D0F097581928ACC.jpg");
                msg += cq::message::MessageSegment::text("👴出错了，将从本地已有的随便发一张（极度敷衍）");
                info.send_back(std::to_string(msg));
                //random_compensate_(arts);
                
                int64_t msg_id = info.send_back(format_result_(arts, info.get_user_id(), no_image, no_detail));
                message_queues_[info.get_group_id()].push(msg_id);
                cq::logging::debug("msg_id", std::to_string(msg_id));
                return eExecuteResult::USER_ERROR;
            }

            if (arts.empty())
            {
                cq::message::Message msg;
                msg += cq::message::MessageSegment::at(info.get_user_id());
                info.send_back("👴啥也没查到");
                return eExecuteResult::SUCCESS;
            }

            int64_t msg_id = info.send_back(format_result_(arts, info.get_user_id(), no_image, no_detail));
            message_queues_[info.get_group_id()].push(msg_id);
            cq::logging::debug("msg_id", std::to_string(msg_id));
            return eExecuteResult::SUCCESS;
        }

    private:
        std::map<int64_t, std::queue<int64_t>> &message_queues_;
        util::Database &index_db_;
        util::Database &group_index_db_;
        util::Database &local_index_db_;

        bool insert_info_( const IllustrationInfo &info ) const
        {
            auto &logger = quinbot::bot.get_logger();
            int32_t id = 0;
            char *err_msg = nullptr;
            if (index_db_.size(id, "PIC_INDEX", err_msg))
            {
                ++id;
                logger.debug("CURRENT_ID", std::to_string(id));
            }
            else
            {
                logger.error("SQLite", "获取PIC_INDEX大小失败");
            }
            return index_db_.insert("PIC_INDEX", "ID, AUTHOR, UID, TITLE, PID, URL, LOCAL_NAME", "?, ?, ?, ?, ?, ?, ?", [&]( sqlite3_stmt *stmt ) -> bool {
                sqlite3_bind_int(stmt, 1, id);
                sqlite3_bind_text(stmt, 2, info.artist.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_int64(stmt, 3, info.uid);
                sqlite3_bind_text(stmt, 4, info.title.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_int64(stmt, 5, info.pid);
                sqlite3_bind_text(stmt, 6, info.url.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 7, info.local_file_name.c_str(), -1, SQLITE_TRANSIENT);
                return true;
            });
        }

        inline std::string format_result_( const std::vector<IllustrationInfo> &arts, int64_t user_id, bool no_image, bool no_detail ) const
        {
            cq::logging::debug("no-image", std::to_string(no_image));
            cq::logging::debug("no-detail", std::to_string(no_detail));
            cq::message::Message msg;

            msg += cq::message::MessageSegment::at(user_id);
            for (const auto &info : arts)
            {
                if (!no_image)
                    msg += cq::message::MessageSegment::image(info.local_file_name);
                if (!no_detail)
                {
                    msg += cq::message::MessageSegment::text(u8"「作者: " + info.artist + u8" UID: " + std::to_string(info.uid) + u8"」\n");
                    msg += cq::message::MessageSegment::text(u8"「作品名: " + info.title + u8" PID: " + std::to_string(info.pid) + u8"」\n");
                    msg += cq::message::MessageSegment::text(info.url + "\n");
                }
            }
            return std::to_string(msg);
        }

        inline std::string format_result_group_info_( const std::vector<GroupIllustrationInfo> &arts, int64_t user_id, bool no_image, bool no_detail ) const
        {
            cq::logging::debug("no-image", std::to_string(no_image));
            cq::logging::debug("no-detail", std::to_string(no_detail));
            cq::message::Message msg;

            msg += cq::message::MessageSegment::at(user_id);
            for (const auto &info : arts)
            {
                if (!no_detail)
                {
                    msg += cq::message::MessageSegment::text("\n「来自:" + info.from + "(" + std::to_string(info.user_id) + ")」\n");
                    msg += cq::message::MessageSegment::text("「添加时间: " + info.add_time + "」\n");
                }
                if (!no_image)
                    msg += cq::message::MessageSegment::image(info.local_file_name);
            }
            return std::to_string(msg);
        }

        IllustrationInfo parse_dict_( PyObject *p_dict ) const
        {
            IllustrationInfo result;

            result.uid = util::pydict_as_int64(p_dict, "uid");
            result.artist = util::pydict_as_string(p_dict, "artist");
            result.pid = util::pydict_as_int64(p_dict, "pid");
            result.title = util::pydict_as_string(p_dict, "title");
            result.url = util::pydict_as_string(p_dict, "url");
            result.local_file_name = util::pydict_as_string(p_dict, "local_file_name");
            return result;
        }

        bool random_compensate_( std::vector<IllustrationInfo> &result ) const
        {
            std::string sentence = "SELECT * FROM PIC_INDEX ORDER BY RANDOM() LIMIT 1";
            char *err_msg = nullptr;
            return index_db_.execute(sentence, [](void *data, int row_count, char **rows, char **cols) -> int {
                std::vector<IllustrationInfo> &result = *reinterpret_cast<std::vector<IllustrationInfo> *>(data);
                IllustrationInfo info;
                for (int i = 0; i < row_count; ++i)
                {
                    std::string row = rows[i];
                    std::string col = cols[i];
                    if (col == "AUTHOR")
                        info.artist = row;
                    else if (col == "UID")
                        info.uid = std::stoi(row);
                    else if (col == "TITLE")
                        info.title = row;
                    else if (col == "PID")
                        info.pid = std::stoi(row);
                    else if (col == "URL")
                        info.url = row;
                    else if (col == "LOCAL_NAME")
                        info.local_file_name = row;
                }
                result.push_back(info);
                return 0;
            }, &result, err_msg);
        }

        bool crop_image_( std::string file_name ) const
        {
            std::string path = "E:/yandere/data/images/" + file_name;
            std::string out = cq::dir::root() + "data/image/temp/local/" + file_name;
            auto &modules = quinbot::bot.get_python_modules();

            PyObject *p_args = PyTuple_New(2);
            PyObject *p_arg1 = PyUnicode_FromString(path.c_str());
            PyObject *p_arg2 = PyUnicode_FromString(out.c_str());
            PyTuple_SetItem(p_args, 0, p_arg1);
            PyTuple_SetItem(p_args, 1, p_arg2);
            bool success = false;

            int state_check = PyGILState_Check();
	        PyGILState_STATE gstate;
	        if (!state_check)
		        gstate = PyGILState_Ensure();

            auto &p_module = modules["random_illustration"];

            if (!p_module)
                quinbot::bot.get_logger().error("Python", "random_illustration模块加载失败");
            else
            {
                PyObject *p_process_func = PyObject_GetAttrString(p_module, "crop_image_out");
                PyObject *p_ret = PyObject_CallObject(p_process_func, p_args);
                if (!p_ret)
                {
                    quinbot::bot.get_logger().warning("Python", "Python调用crop_image时发生异常");
                }
                else
                    success = true;
            }

            Py_BEGIN_ALLOW_THREADS;
            Py_BLOCK_THREADS;

            Py_UNBLOCK_THREADS;
	        Py_END_ALLOW_THREADS;
            if (!state_check)
                PyGILState_Release(gstate);
            
            return success;
        }

        bool get_local_illustration_( std::vector<std::string> &result, int32_t num, std::string keyword, int32_t score ) const
        {
            std::string sentence;
            if (!keyword.empty())
                sentence = "SELECT * FROM PIC_INDEX WHERE TAGS GLOB '*" + keyword + "*'" + "ORDER BY RANDOM() LIMIT " + std::to_string(num);
            else 
                sentence = "SELECT * FROM PIC_INDEX ORDER BY RANDOM() LIMIT " + std::to_string(num);
            char *err_msg = nullptr;
            return local_index_db_.execute(sentence, [](void *data, int row_count, char **rows, char **cols) -> int {
                std::vector<std::string> &result = *reinterpret_cast<std::vector<std::string> *>(data);
                std::string info;
                for (int i = 0; i < row_count; ++i)
                {
                    std::string row = rows[i];
                    std::string col = cols[i];
                    if (col == "FILENAME")
                    {
                        info = row;
                        break;
                    }
                }
                result.push_back("temp/local/" + info);
                return 0;
            }, &result, err_msg);
        }

        bool get_all_group_image_( std::vector<GroupIllustrationInfo> &result, int64_t group_user_id = 0 ) const
        {
            std::string sentence = "SELECT * FROM GROUP_PIC_INDEX WHERE USER_ID = " + std::to_string(group_user_id);
            char *err_msg = nullptr;
            return group_index_db_.execute(sentence, [](void *data, int row_count, char **rows, char **cols) -> int {
                std::vector<GroupIllustrationInfo> &result = *reinterpret_cast<std::vector<GroupIllustrationInfo> *>(data);
                GroupIllustrationInfo info;
                for (int i = 0; i < row_count; ++i)
                {
                    std::string row = rows[i];
                    std::string col = cols[i];
                    if (col == "ADD_BY")
                        info.from = row;
                    else if (col == "USER_ID")
                        info.user_id = std::stoll(row);
                    else if (col == "ADD_TIME")
                        info.add_time = row;
                    else if (col == "LOCAL_NAME")
                        info.local_file_name = row;
                }
                result.push_back(info);
                return 0;
            }, &result, err_msg);
        }

        bool random_group_compensate_( int32_t num, std::vector<GroupIllustrationInfo> &result, int64_t group_user_id = 0 ) const
        {
            std::string sentence;
            if (group_user_id != 0)
                sentence = "SELECT * FROM GROUP_PIC_INDEX WHERE USER_ID = " + std::to_string(group_user_id) +  " ORDER BY RANDOM() LIMIT " + std::to_string(num);
            else
                sentence = "SELECT * FROM GROUP_PIC_INDEX ORDER BY RANDOM() LIMIT " + std::to_string(num);
            char *err_msg = nullptr;
            return group_index_db_.execute(sentence, [](void *data, int row_count, char **rows, char **cols) -> int {
                std::vector<GroupIllustrationInfo> &result = *reinterpret_cast<std::vector<GroupIllustrationInfo> *>(data);
                GroupIllustrationInfo info;
                for (int i = 0; i < row_count; ++i)
                {
                    std::string row = rows[i];
                    std::string col = cols[i];
                    if (col == "ADD_BY")
                        info.from = row;
                    else if (col == "USER_ID")
                        info.user_id = std::stoll(row);
                    else if (col == "ADD_TIME")
                        info.add_time = row;
                    else if (col == "LOCAL_NAME")
                        info.local_file_name = row;
                }
                result.push_back(info);
                return 0;
            }, &result, err_msg);
        }

        bool get_illustration_info_( std::vector<IllustrationInfo> &result, int32_t r18 = 0, int32_t num = 1, const std::string &keyword = "-", const std::string &proxy = "i.pixiv.cat" ) const
        {
            auto &logger = quinbot::bot.get_logger();
            auto &python_modules = quinbot::bot.get_python_modules();
            bool success = false;
            result.clear();
            PyObject *p_args = PyTuple_New(4);
            PyObject *p_arg1 = PyLong_FromLong(r18);
            PyObject *p_arg2 = PyLong_FromLong(num);
            PyObject *p_arg3 = PyUnicode_FromString(proxy.c_str());
            PyObject *p_arg4 = PyUnicode_FromString(keyword.c_str());
            PyTuple_SetItem(p_args, 0, p_arg1);
            PyTuple_SetItem(p_args, 1, p_arg2);
            PyTuple_SetItem(p_args, 2, p_arg3);
            PyTuple_SetItem(p_args, 3, p_arg4);

            int state_check = PyGILState_Check();
	        PyGILState_STATE gstate;
	        if (!state_check)
		        gstate = PyGILState_Ensure();

            Py_BEGIN_ALLOW_THREADS;
            Py_BLOCK_THREADS;

            auto &p_module = python_modules["random_illustration"];

            if (!p_module)
                quinbot::bot.get_logger().error("Python", "random_illustration模块加载失败");
            else
            {
                PyObject *p_process_func = PyObject_GetAttrString(p_module, "get_illustration_info");
                PyObject *p_ret = PyObject_CallObject(p_process_func, p_args);
                if (!p_ret)
                    quinbot::bot.get_logger().warning("Python", "random_illustration模块调用时发生异常");
                else
                {
                    Py_ssize_t list_size = PyList_Size(p_ret);

                    if (list_size == 1)
                    {
                        std::string err_msg = util::pydict_as_string(PyList_GetItem(p_ret, 0), "error_message");
                        if (!err_msg.empty())
                        {
                            quinbot::bot.get_logger().warning("Python", "Python调用时发生异常");
                            quinbot::bot.get_logger().debug("Python错误信息", err_msg);
                        }
                        else
                        {
                            for (int i = 0; i < list_size; ++i)
                            {
                                result.push_back(parse_dict_(PyList_GetItem(p_ret, i)));
                                if (!insert_info_(result.back()))
                                    logger.error("SQLite", "插入数据失败!");
                            }
                            success = true;
                        }
                    }
                    else
                    {
                        for (int i = 0; i < list_size; ++i)
                        {
                            result.push_back(parse_dict_(PyList_GetItem(p_ret, i)));
                            if (!insert_info_(result.back()))
                                logger.error("SQLite", "插入数据失败!");
                        }
                        success = true;
                    }
                }
            }

            if (!success)
            {
                random_compensate_(result);
                std::string full_path = cq::dir::root() + "data/image/" + result[0].local_file_name;
                PyObject *p_args = PyTuple_New(1);
                PyObject *p_arg1 = PyUnicode_FromString(full_path.c_str());
                PyTuple_SetItem(p_args, 0, p_arg1);
                PyObject *p_process_func = PyObject_GetAttrString(p_module, "crop_image");
                PyObject *p_ret = PyObject_CallObject(p_process_func, p_args);
                if (!p_ret)
                {
                    quinbot::bot.get_logger().warning("Python", "Python调用crop_image时发生异常");
                }
            }

            Py_UNBLOCK_THREADS;
	        Py_END_ALLOW_THREADS;
            if (!state_check)
                PyGILState_Release(gstate);

            return success;
        }

    };

    class RandomIllustrationPlugin final : public Plugin
    {
    public:
        RandomIllustrationPlugin()
            :   Plugin("random_illustration")
        {}

        void on_enable() override
        {
            auto &manager = bot.get_command_manager();
            auto &logger = bot.get_logger();
            if (index_db_.open(cq::dir::root() + "/data/image/temp/pixiv/pic_index.db", SQLITE_OPEN_READWRITE | SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_SHAREDCACHE))
                logger.info("SQLite", "数据库pic_index打开成功");
            else
                logger.error("SQLite", "数据库pic_index打开失败");

            if (group_index_db_.open(cq::dir::app() + "/group_pic_index.db", SQLITE_OPEN_READWRITE | SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_SHAREDCACHE))
                logger.info("SQLite", "数据库group_pic_index打开成功");
            else
                logger.error("SQLite", "数据库group_pic_index打开失败");

            if (local_index_db_.open(cq::dir::app() + "/image_index.db", SQLITE_OPEN_READWRITE | SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_SHAREDCACHE))
                logger.info("SQLite", "数据库local_pic_index打开成功");
            else
                logger.error("SQLite", "数据库local_pic_index打开失败");
            manager->register_command<ILoveSexIllustrationCommand>(message_queues_, index_db_, group_index_db_, local_index_db_);
            manager->register_command<RecoverCommand>(message_queues_);
            manager->register_command<DonateCommand>(group_index_db_);
        }

        void on_disable() override
        {
            auto &manager = bot.get_command_manager();
            index_db_.close();
            group_index_db_.close();
            local_index_db_.close();
        }

    private:
        std::map<int64_t, std::queue<int64_t>> message_queues_;
        util::Database index_db_;
        util::Database group_index_db_;
        util::Database local_index_db_;
    };
}
}