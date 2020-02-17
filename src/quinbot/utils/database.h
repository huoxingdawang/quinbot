#pragma once

#include <cstdint>
#include <initializer_list>
#include <functional>
#include <memory>
#include <string>

#include "../cqsdk/logging.h"

#include <sqlite3.h>

namespace quinbot
{
namespace util
{
    class Database
    {
        typedef int (*SqlCallback)( void *, int, char **, char ** );
        using SqlBinder = std::function<bool ( sqlite3_stmt * )>;
#define CHECK_OPEN() if (!check_open_()) { return false; }

    public:
        Database()
            :   p_db_(nullptr)
        {

        }

        inline bool open( const std::string &path, int32_t flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_SHAREDCACHE )
        {
            if (p_db_)
                close();
            int32_t open_result = sqlite3_open_v2(path.c_str(), &p_db_, flags, NULL);
            return open_result == SQLITE_OK;
        }

        inline bool close()
        {
            if (!p_db_)
                return false;
            sqlite3_close(p_db_);
            p_db_ = nullptr;
            return true;
        }

        Database( const Database &other ) = delete;
        Database *operator=( const Database &other ) = delete;

        inline bool execute( const std::string &sentence, SqlCallback callback, void *data = nullptr, char *err_msg = nullptr ) const
        {
            CHECK_OPEN();
            int32_t result = sqlite3_exec(p_db_, sentence.c_str(), callback, data, &err_msg);
            return result == SQLITE_OK;
        }

        bool execute( const std::string &sentence ) const
        {
            CHECK_OPEN();
            sqlite3_stmt *stmt;
            bool result = false;
            int32_t prepare_result = sqlite3_prepare_v2(p_db_, sentence.c_str(), -1, &stmt, nullptr);
            if (prepare_result == SQLITE_OK)
            {
                int32_t step_result = sqlite3_step(stmt);
                if (step_result == SQLITE_DONE)
                    result = true;
            }
            sqlite3_finalize(stmt);
            return result;
        }

        inline bool create_table( const std::string &name, const std::string &detail ) const
        {
            CHECK_OPEN();
            std::string sentence = "CREATE TABLE " + name + " (" + detail + ")";
            return execute(sentence);
        }

        inline bool drop_table( const std::string &name ) const
        {
            CHECK_OPEN();
            std::string sentence = "DROP TABLE " + name;
            return execute(sentence);
        }

        inline bool insert( const std::string &table_name, const std::string &keys, const std::string &values ) const
        {
            CHECK_OPEN();
            std::string sentence = "INSERT INTO " + table_name + " (" + keys + ") VALUES (" + values + ")";
            return execute(sentence);
        }

        bool insert( const std::string &table_name, const std::string &keys, const std::string &values, SqlBinder binder, char *err_msg = nullptr ) const
        {
            CHECK_OPEN();
            std::string sentence = "INSERT INTO " + table_name + " (" + keys + ") VALUES (" + values + ")";
            
            bool result = false;
            sqlite3_stmt *stmt = nullptr;
            int32_t prepare_result = sqlite3_prepare_v2(p_db_, sentence.c_str(), -1, &stmt, nullptr);
            if (prepare_result == SQLITE_OK)
            {
                if (binder(stmt))
                {
                    int32_t step_result = sqlite3_step(stmt);
                    if (step_result == SQLITE_DONE)
                        result = true;
                }
            }
            sqlite3_finalize(stmt);
            return result;
        }

        inline bool select( const std::string &table_name, const std::string &keys, const std::string &condition, SqlCallback callback, void *result_out, char *err_msg = nullptr ) const
        {
            CHECK_OPEN();
            std::string sentence = "SELECT " + keys + " FROM " + table_name + " WHERE " + condition;
            return execute(sentence, callback, result_out, err_msg);
        }

        inline bool select( const std::string &table_name, const std::string &condition, SqlCallback callback, void *result_out, char *err_msg = nullptr ) const
        {
            CHECK_OPEN();
            std::string sentence = "SELECT * FROM " + table_name + " WHERE " + condition;
            return execute(sentence, callback, result_out, err_msg);
        }

        inline bool remove( const std::string &table_name, const std::string &condition ) const
        {
            CHECK_OPEN();
            std::string sentence = "DELETE FROM " + table_name + " WHERE " + condition;
            return execute(sentence);
        }

        bool update( const std::string &table_name, std::initializer_list<std::pair<std::string, std::string>> key_value_pairs, const std::string &condition ) const
        {
            CHECK_OPEN();
            std::string sentence = "UPDATE " + table_name + " SET ";
            for (auto it = key_value_pairs.begin(); it != key_value_pairs.end(); ++it)
            {
                sentence += it->first + " = " + it->second + ",";
            }
            sentence.back() = ' ';
            sentence += "WHERE " + condition;
            return execute(sentence);
        }

        inline bool size( int32_t &result, const std::string &table_name, char *err_msg = nullptr ) const
        {
            CHECK_OPEN();
            std::string sentence = "SELECT COUNT(*) FROM " + table_name;
            int32_t count = 0;
            bool exec_result = execute(sentence, []( void *data, int row_count, char **rows, char **cols ) -> int {
                if (row_count == 1 && rows)
                {
                    *static_cast<int32_t *>(data) = std::atoi(rows[0]);
                    return 0;
                }
                return 1;
            }, &count, err_msg);
            result = count;
            return exec_result;
        }

        inline bool size( int32_t &result, const std::string &table_name, const std::string &col, const std::string &row, char *err_msg = nullptr ) const
        {
            CHECK_OPEN();
            std::string sentence = "SELECT COUNT(*) FROM " + table_name + " WHERE " + col + " = " + row;
            int32_t count = 0;
            bool exec_result = execute(sentence, []( void *data, int row_count, char **rows, char **cols ) -> int {
                if (row_count == 1 && rows)
                {
                    *static_cast<int32_t *>(data) = std::atoi(rows[0]);
                    return 0;
                }
                return 1;
            }, &count, err_msg);
            result = count;
            return exec_result;
        }

        inline bool table_exists( bool &result, const std::string &table_name, char *err_msg = nullptr ) const
        {
            CHECK_OPEN();
            std::string sentence = "SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='" + table_name + "'";
            int32_t count = 0;
            bool exec_result = execute(sentence, []( void *data, int row_count, char **rows, char **cols ) -> int {
                if (row_count == 1 && rows)
                {
                    *static_cast<int32_t *>(data) = std::atoi(rows[0]);
                    return 0;
                }
                return 1;
            }, &count, err_msg);
            result = count == 0 ? false : true;
            return exec_result;
        }

    private:
        sqlite3 *p_db_;

        inline bool check_open_() const { return p_db_ != nullptr; cq::logging::error("not_open", "!"); }

    };
}
}