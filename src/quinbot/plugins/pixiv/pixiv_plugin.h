#pragma once

#include "../../plugin.h"
#include "../../bot.h"

#define BAD_USER_ERROR() throw std::exception("USER_ERROR");

namespace pixiv
{
    using namespace quinbot;
    struct WorkInfo
    {
        int32_t rank;
        std::string author;
        int64_t uid;
        std::string title;
        int64_t pid;
        std::string local_file_name;
        std::string url;
        std::string tags;
        int64_t date;
    };

    int32_t inner_to_int32( const std::string &str, bool omit = false )
    {
        int32_t ret = 0;
        for (int i = 0; str[i]; ++i)
        {
            if (str[i] >= '0' && str[i] <= '9')
                ret = ret * 10 + str[i] - '0';
            else
            {
                if (!omit)
                    return -1;
            }
                
        }
        return ret;
    }

    int32_t inner_to_int64( const std::string &str, bool omit = false )
    {
        int64_t ret = 0;
        for (int i = 0; str[i]; ++i)
        {
            if (str[i] >= '0' && str[i] <= '9')
                ret = ret * 10 + str[i] - '0';
            else
            {
                if (!omit)
                    return -1;
            }
                
        }
        return ret;
    }

    bool is_legal_date( const std::string &date )
    {
        static int32_t everyday[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        if (date.size() != 10)
            return false;
        int32_t year = inner_to_int32(std::string(date, 0, 4));
        int32_t month = inner_to_int32(std::string(date, 5, 2));
        int32_t day = inner_to_int32(std::string(date, 8, 2));
        if (year == -1 || month == -1 || day == -1 || month > 12 || day > 31)
            return false;
        auto [t_year, t_month, t_day] = util::date_today();
        if (year - 1 > t_year || month - 1 > t_month || day - 1 > t_day)
            return false;
        if (month == 2 && day == 29 && !(year % 4 == 0 && year % 100 == 0 || year % 400 == 0))
            return false;
        if (day > everyday[month - 1])
            return false;
        return true;
    }

    WorkInfo parse_dict( PyObject *p_dict )
    {
        WorkInfo result;
        result.rank = util::pydict_as_int32(p_dict, "rank");
        result.uid = util::pydict_as_int64(p_dict, "uid");
        result.author = util::pydict_as_string(p_dict, "author");
        result.pid = util::pydict_as_int64(p_dict, "pid");
        result.title = util::pydict_as_string(p_dict, "title");
        result.url = util::pydict_as_string(p_dict, "url");
        result.local_file_name = util::pydict_as_string(p_dict, "local_file_name");
        result.tags = util::pydict_as_string(p_dict, "tags");
        return result;
    }

    bool insert_ranking_info( util::Database &db, const std::string &table_name, const WorkInfo &info )
    {
        auto &logger = quinbot::bot.get_logger();
        #define qdi(name, var) cq::logging::debug(name, std::to_string(info.##var));
        #define qds(name, var) cq::logging::debug(name, info.##var);
        qdi("rank", rank);
        qds("author", author);
        qdi("uid", uid);
        qds("title", title);
        qdi("pid", pid);
        qds("url", url);
        qds("local_file_name", local_file_name);
        #undef qdi
        #undef qds

        static const std::string keys = "RANK, AUTHOR, UID, TITLE, PID, TAGS, URL, LOCAL_NAME, DATE";
        static const std::string values = "?, ?, ?, ?, ?, ?, ?, ?, ?";
        return db.insert(table_name, keys, values, [&]( sqlite3_stmt *stmt ) -> bool {
            sqlite3_bind_int(stmt, 1, info.rank);
            sqlite3_bind_text(stmt, 2, info.author.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int64(stmt, 3, info.uid);
            sqlite3_bind_text(stmt, 4, info.title.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int64(stmt, 5, info.pid);
            sqlite3_bind_text(stmt, 6, info.tags.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 7, info.url.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 8, info.local_file_name.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int64(stmt, 9, info.date);
            return true;
        });
    }

    inline bool insert_cache_info( util::Database &db, const std::string &table_name, const WorkInfo &info )
    {
        static const std::string keys = "AUTHOR, UID, TITLE, PID, TAGS, URL, LOCAL_NAME";
        static const std::string values = "?, ?, ?, ?, ?, ?, ?";
        return db.insert(table_name, keys, values, [&]( sqlite3_stmt *stmt ) -> bool {
            sqlite3_bind_text(stmt, 1, info.author.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int64(stmt, 2, info.uid);
            sqlite3_bind_text(stmt, 3, info.title.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int64(stmt, 4, info.pid);
            sqlite3_bind_text(stmt, 5, info.tags.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 6, info.url.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 7, info.local_file_name.c_str(), -1, SQLITE_TRANSIENT);
            return true;
        });
    }

    inline bool create_ranking_table( util::Database &db, const std::string &table_name )
    {
        return db.create_table(table_name,
            "RANK INT PRIMARY KEY UNIQUE NOT NULL,\n"
            "AUTHOR TEXT NOT NULL,\n"
            "UID INT NOT NULL,\n"
            "TITLE TEXT NOT NULL,\n"
            "PID INT NOT NULL,\n"
            "TAGS TEXT NOT NULL,\n"
            "URL TEXT NOT NULL,\n"
            "LOCAL_NAME TEXT NOT NULL\n"
            "DATE BIGINT NOT NULL"
        );
    }

    inline bool create_cache_table( util::Database &db, const std::string &table_name )
    {
        return db.create_table(table_name,
            "AUTHOR TEXT NOT NULL,\n"
            "UID INT NOT NULL,\n"
            "TITLE TEXT NOT NULL,\n"
            "PID INT UNIQUE NOT NULL,\n"
            "TAGS TEXT NOT NULL,\n"
            "URL TEXT NOT NULL,\n"
            "LOCAL_NAME TEXT NOT NULL"
        );
    }

    bool query_ranking( util::Database &db, std::vector<WorkInfo> &result, const std::string &table_name, const std::string &need = "" )
    {
        const std::string sentence = "SELECT * FROM " + table_name + " " + need;
        char *err_msg = nullptr;
        return db.execute(sentence, [](void *data, int row_count, char **rows, char **cols) -> int {
            std::vector<WorkInfo> &result = *reinterpret_cast<std::vector<WorkInfo> *>(data);
            WorkInfo info;
            for (int i = 0; i < row_count; ++i)
            {
                std::string row = rows[i];
                std::string col = cols[i];
                if (col == "RANK")
                    info.rank = std::stoi(row);
                else if (col == "UID")
                    info.uid = std::stoll(row);
                else if (col == "AUTHOR")
                    info.author = row;
                else if (col == "PID")
                    info.pid = std::stoll(row);
                else if (col == "TITLE")
                    info.title = row;
                else if (col == "TAGS")
                    info.tags = row;
                else if (col == "URL")
                    info.url = row;
                else if (col == "LOCAL_NAME")
                    info.local_file_name = row;
                else if (col == "DATE")
                    info.date = std::stoll(row);
            }
            result.push_back(info);
            return 0;
        }, &result, err_msg);
    }

    bool query_cache( util::Database &db, std::vector<WorkInfo> &result, const std::string &table_name, const std::string &need = "" )
    {
        const std::string sentence = "SELECT * FROM " + table_name + need;
        char *err_msg = nullptr;
        return db.execute(sentence, [](void *data, int row_count, char **rows, char **cols) -> int {
            std::vector<WorkInfo> &result = *reinterpret_cast<std::vector<WorkInfo> *>(data);
            WorkInfo info;
            for (int i = 0; i < row_count; ++i)
            {
                std::string row = rows[i];
                std::string col = cols[i];
                if (col == "UID")
                    info.uid = std::stoll(row);
                else if (col == "AUTHOR")
                    info.author = row;
                else if (col == "PID")
                    info.pid = std::stoll(row);
                else if (col == "TITLE")
                    info.title = row;
                else if (col == "TAGS")
                    info.tags = row;
                else if (col == "URL")
                    info.url = row;
                else if (col == "LOCAL_NAME")
                   info.local_file_name = row;
            }
            result.push_back(info);
            return 0;
        }, &result, err_msg);
    }
}

namespace quinbot
{
namespace plugin
{
    class PixivAuthorSearchCommand final : public command::PublicCommand
    {
        using eExecuteResult = command::eExecuteResult;
        using CommandInfo =  command::CommandInfo;
    public:
        PixivAuthorSearchCommand( util::Database &db )
            :   command::PublicCommand("Apixiv_uid"),
                db_(db)
        {
            set_aliases({"pixiv_uid_search", "p站作者", "P站作者", "pixiv作者"});
        }

        eExecuteResult process( const CommandInfo &info ) override
        {
            auto &cl = info.command_line;
            auto &logger = quinbot::bot.get_logger();
            std::tuple<int64_t, int32_t, bool, bool> init_result;

            try { init_result = init_args_(info); } catch (...) { return eExecuteResult::USER_ERROR; }
            auto [uid, count, no_detail, no_image] = init_result;

            if (no_image && no_detail)
            {
                info.send_back("什么都不要你跟我说你🐴呢");
                return eExecuteResult::SUCCESS;
            }

            std::vector<pixiv::WorkInfo> works;
            if (!pixiv_author_search_pycall_(works, uid, count))
            {
                info.send_back("wrong");
            }
            else
            {
                info.send_back(format_result_(works, info.get_user_id(), no_detail, no_image));
            }

            return eExecuteResult::SUCCESS;
        }

    private:
        util::Database &db_;

        std::tuple<int64_t, int32_t, bool, bool> init_args_( const CommandInfo &info ) const
        {
            const auto &cl = info.command_line;
            command::ArgsMap args = command::arg_parse(cl, 
            {
                {"uid", {"用户ID", "用户id"}},
                {"count", {"数量", "数目"}},
            },
            {
                {"no_detail", {"no-detail", "不显示详细", "不显示详情"}},
                {"no_image", {"no-image", "不显示图片"}}
            }
            );

            if (args.is_bad())
            {
                info.send_back(args.error_message);
                BAD_USER_ERROR();
            }
            
            int64_t uid = 0;
            int32_t count = 0;

            try
            {
                uid = args.get<int64_t>("uid", 0, true);
                count = args.get<int32_t>("count", 1, false);
            }
            catch ( const std::underflow_error & )
            {
                info.send_back("有效参数过少");
                BAD_USER_ERROR();
            }
            catch ( const std::invalid_argument & )
            {
                info.send_back("错误的参数解析");
                BAD_USER_ERROR();
            }

            bool no_detail = args.get_option("no_detail");
            bool no_image = args.get_option("no_image");

            return {uid, count, no_detail, no_image};
        }

        bool pixiv_author_search_pycall_( std::vector<pixiv::WorkInfo> &result, int64_t uid, int32_t count )
        {
            auto &logger = quinbot::bot.get_logger();
            auto &python_modules = quinbot::bot.get_python_modules();
            bool success = false;
            result.clear();
            PyObject *p_args = PyTuple_New(2);
            PyObject *p_arg1 = PyLong_FromLongLong(uid);
            PyObject *p_arg2 = PyLong_FromLong(count);
            PyTuple_SetItem(p_args, 0, p_arg1);
            PyTuple_SetItem(p_args, 1, p_arg2);

            int state_check = PyGILState_Check();
	        PyGILState_STATE gstate;
	        if (!state_check)
		        gstate = PyGILState_Ensure();

            Py_BEGIN_ALLOW_THREADS;
            Py_BLOCK_THREADS;

            auto &p_module = python_modules["pixiv"];
            if (!p_module)
                quinbot::bot.get_logger().error("Python", "pixiv模块加载失败");
            else
            {
                PyObject *p_process_func = PyObject_GetAttrString(p_module, "pixiv_search_by_uid");
                PyObject *p_ret = PyObject_CallObject(p_process_func, p_args);
                if (!p_ret)
                    logger.warning("Python", "pixiv模块调用时发生异常");
                else
                {
                    Py_ssize_t list_size = PyList_Size(p_ret);
                    std::string err_msg;
                    if (list_size == 1)
                        err_msg = util::pydict_as_string(PyList_GetItem(p_ret, 0), "error_message");
                    if (!err_msg.empty())
                    {
                        logger.warning("Python", "Python调用时发生异常");
                        logger.debug("Python错误信息", err_msg);
                    }
                    else
                    {
                        for (int i = 0; i < list_size; ++i)
                        {
                            auto info = pixiv::parse_dict(PyList_GetItem(p_ret, i));
                            result.push_back(info);
                            pixiv::insert_cache_info(db_, "PIXIV_CACHE", result.back());
                            //    logger.warning("SQLite", "插入数据失败!");
                        }
                        success = true;
                    }   
                }
            }

            Py_UNBLOCK_THREADS;
	        Py_END_ALLOW_THREADS;
            if (!state_check)
                PyGILState_Release(gstate);
            return success;
        }

        std::string format_result_( const std::vector<pixiv::WorkInfo> &works, int64_t user_id, bool no_detail, bool no_image ) const
        {
            cq::message::Message msg;
            msg += cq::message::MessageSegment::at(user_id);
            for (const auto &info : works)
            {
                if (!no_image)
                    msg += cq::message::MessageSegment::image(info.local_file_name);
                if (!no_detail)
                {
                    msg += cq::message::MessageSegment::text(u8"「作者: " + info.author + u8" UID: " + std::to_string(info.uid) + u8"」\n");
                    msg += cq::message::MessageSegment::text(u8"「作品名: " + info.title + u8" PID: " + std::to_string(info.pid) + u8"」\n");
                    msg += cq::message::MessageSegment::text(info.url + "\n");
                }
            }
            return std::to_string(msg);
        }
    };

    class PixivRankingSearchCommand final : public command::PublicCommand
    {
        using eExecuteResult = command::eExecuteResult;
        using CommandInfo =  command::CommandInfo;
    public:
        PixivRankingSearchCommand( util::Database &db )
            :   command::PublicCommand("Apixiv_rank"),
                db_(db)
        {
            set_aliases({"pixiv_rank_search", "p站排名", "P站排名", "pixiv排名"});
        }

        eExecuteResult process( const CommandInfo &info ) override
        {
            auto &cl = info.command_line;
            auto &logger = quinbot::bot.get_logger();

            std::tuple<int32_t, std::string, std::string, int64_t, bool, bool> init_result;
            try { init_result = init_args_(info); } catch(...) { return eExecuteResult::USER_ERROR; }
            auto [top, mode, date, date_int, no_detail, no_image] = init_result;

            std::string table_name = mode;
            cq::logging::debug("table_name", table_name);

            if (no_image && no_detail)
            {
                info.send_back("什么都不要你跟我说你🐴呢");
                return eExecuteResult::SUCCESS;
            }

            int32_t size = 0;
            char *err_msg = nullptr;
            if (!db_.size(size, table_name, "DATE", std::to_string(date_int), err_msg))
            {
                logger.error("SQLite", "查询表大小失败");
                if (err_msg)
                    logger.debug("SQLite错误信息", std::string(err_msg));
            }
                    
            if (size >= top)
            {
                std::vector<pixiv::WorkInfo> works;
                pixiv::query_ranking(db_, works, table_name, "WHERE DATE = " + std::to_string(date_int) + " ORDER BY RANK LIMIT " + std::to_string(top));
                info.send_back(format_result_(works, info.get_user_id(), no_detail, no_image));
                return eExecuteResult::SUCCESS;
            }
            
            std::vector<pixiv::WorkInfo> works;
            if (!pixiv_ranking_search_pycall_(works, top, mode, date, date_int, size))
            {
                info.send_back("wrong");
            }
            else
            {
                info.send_back(format_result_(works, info.get_user_id(), no_detail, no_image));
            }

            return eExecuteResult::SUCCESS;
        }

    private:
        util::Database &db_;

        std::tuple<int32_t, std::string, std::string, int64_t, bool, bool> init_args_( const command::CommandInfo &info ) const
        {
            const auto &cl = info.command_line;
            command::ArgsMap args = command::arg_parse(cl, 
            {
                {"top", {"排名"}},
                {"mode", {"模式", "类型"}},
                {"date", {"时间"}}
            },
            {
                {"no_detail", {"no-detail", "不显示详细", "不显示详情"}},
                {"no_image", {"no-image", "不显示图片"}}
            }
            );

            if (args.is_bad())
            {
                info.send_back(args.error_message);
                BAD_USER_ERROR();
            }
            std::string top_str = args.get<std::string>("top", "top5", false);
            if (!util::in(top_str, {"top5", "top10", "top15", "top20"}))
            {
                info.send_back("不支持的top选项");
                BAD_USER_ERROR();
            }
            int32_t top = std::stoi(std::string(top_str, 3));

            std::string mode = args.get<std::string>("mode", "daily", false);
            if (!util::in(mode, {"daily", "weekly", "monthly", "male", "female", "daily_r18", "weekly_r18", "male_r18", "female_r18", "original", "rookie"}))
            {
                info.send_back("不支持的mode选项");
                BAD_USER_ERROR();
            }

            std::string date = args.get<std::string>("date", "", false);
            if (!date.empty() && !pixiv::is_legal_date(date))
            {
                info.send_back("错误的时间格式或时间超过今日");
                BAD_USER_ERROR();
            }

            if (date.empty())
            {
                auto [y, m, d] = util::date_today();
                date = util::date_format_str(y, m, d - 2);
            }

            int64_t date_int = pixiv::inner_to_int64(date, true);

            bool no_detail = args.get_option("no_detail");
            bool no_image = args.get_option("no_image");

            cq::logging::debug("query_time", date + " " + std::to_string(date_int));
            return {top, mode, date, date_int, no_detail, no_image};
        }

        std::string format_result_( const std::vector<pixiv::WorkInfo> &works, int64_t user_id, bool no_detail, bool no_image ) const
        {
            cq::message::Message msg;
            msg += cq::message::MessageSegment::at(user_id);
            for (const auto &info : works)
            {
                if (!no_image)
                    msg += cq::message::MessageSegment::image(info.local_file_name);
                if (!no_detail)
                {
                    msg += cq::message::MessageSegment::text(u8"「作者: " + info.author + u8" UID: " + std::to_string(info.uid) + u8"」\n");
                    msg += cq::message::MessageSegment::text(u8"「作品名: " + info.title + u8" PID: " + std::to_string(info.pid) + u8"」\n");
                    msg += cq::message::MessageSegment::text(info.url + "\n");
                }
            }
            return std::to_string(msg);
        }

        bool pixiv_ranking_search_pycall_( std::vector<pixiv::WorkInfo> &result, int32_t top, const std::string &mode, const std::string &date, int64_t date_int, int32_t current_size )
        {
            auto &logger = quinbot::bot.get_logger();
            auto &python_modules = quinbot::bot.get_python_modules();
            bool success = false;
            result.clear();
            PyObject *p_args = PyTuple_New(3);
            PyObject *p_arg1 = PyLong_FromLong(top);
            PyObject *p_arg2 = PyUnicode_FromString(mode.c_str());
            PyObject *p_arg3 = PyUnicode_FromString(date.c_str());
            PyTuple_SetItem(p_args, 0, p_arg1);
            PyTuple_SetItem(p_args, 1, p_arg2);
            PyTuple_SetItem(p_args, 2, p_arg3);

            std::string table_name = mode;

            int state_check = PyGILState_Check();
	        PyGILState_STATE gstate;
	        if (!state_check)
		        gstate = PyGILState_Ensure();

            Py_BEGIN_ALLOW_THREADS;
            Py_BLOCK_THREADS;

            auto &p_module = python_modules["pixiv"];
            if (!p_module)
                quinbot::bot.get_logger().error("Python", "pixiv模块加载失败");
            else
            {
                PyObject *p_process_func = PyObject_GetAttrString(p_module, "pixiv_ranking_search");
                PyObject *p_ret = PyObject_CallObject(p_process_func, p_args);
                if (!p_ret)
                    logger.warning("Python", "pixiv模块调用时发生异常");
                else
                {
                    Py_ssize_t list_size = PyList_Size(p_ret);
                    std::string err_msg;
                    if (list_size == 1)
                        err_msg = util::pydict_as_string(PyList_GetItem(p_ret, 0), "error_message");
                    if (!err_msg.empty())
                    {
                        logger.warning("Python", "Python调用时发生异常");
                        logger.debug("Python错误信息", err_msg);
                    }
                    else
                    {
                        for (int i = 0; i < list_size; ++i)
                        {
                            auto info = pixiv::parse_dict(PyList_GetItem(p_ret, i));
                            info.date = date_int;
                            result.push_back(info);
                            if (info.rank <= current_size)
                                continue;
                            if (!pixiv::insert_ranking_info(db_, table_name, result.back()))
                                logger.warning("SQLite", "插入数据失败!");
                        }
                        success = true;
                    }   
                }
            }

            Py_UNBLOCK_THREADS;
	        Py_END_ALLOW_THREADS;
            if (!state_check)
                PyGILState_Release(gstate);
            return success;
        }
    };

    class PixivPlugin final : public Plugin
    {
    public:
        PixivPlugin()
            :   Plugin("pixiv")
        {}

        void on_enable() override
        {
            auto &manager = quinbot::bot.get_command_manager();
            auto &logger = bot.get_logger();
            if (pixiv_ranking_db_.open(cq::dir::app() + "/pixiv_ranking_index.db", SQLITE_OPEN_READWRITE | SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_SHAREDCACHE))
                logger.info("SQLite", "数据库pixiv_ranking_index打开成功");
            else
                logger.error("SQLite", "数据库pixiv_ranking_index打开失败");
            if (pixiv_cache_db_.open(cq::dir::app() + "/pixiv_cache_index.db", SQLITE_OPEN_READWRITE | SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_SHAREDCACHE)) 
                logger.info("SQLite", "数据库pixiv_cache_index打开成功");
            else
                logger.error("SQLite", "数据库pixiv_cache_index打开失败");
            manager->register_command<PixivRankingSearchCommand>(pixiv_ranking_db_);
            manager->register_command<PixivAuthorSearchCommand>(pixiv_cache_db_);
        }

        void on_disable() override
        {
            auto &manager = quinbot::bot.get_command_manager();
            auto &logger = bot.get_logger();
            pixiv_ranking_db_.close();
            pixiv_cache_db_.close();
        }

    private:
        util::Database pixiv_ranking_db_;
        util::Database pixiv_cache_db_;
    };
}
}
