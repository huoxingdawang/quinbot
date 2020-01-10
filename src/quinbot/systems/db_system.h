#ifndef __QUINBOT_DB_SYSTEM_H__
#define __QUINBOT_DB_SYSTEM_H__

#include <string>

#include "../../sqlite3/sqlite3.h"

namespace quinbot
{
namespace system
{
    class Database
    {
    public:
        Database();
        
        Database( const char *path );

        Database( const std::string &path );

        void open( const char *path );

        void open( const std::string &path );

        void close();

        void excute( const char *command );

    private:
        std::string name_;
        std::string path_;
        sqlite3 *db_;

    };

}
}

#endif