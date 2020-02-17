#pragma once

#include <iomanip>
#include <string>
#include <sstream>
#include <filesystem>
#include <fstream>
#include <Windows.h>

#include "cqsdk/logging.h"

#include "enums.h"

namespace quinbot
{
    class InnerLogger
    {
        friend class Logger;
    public:
        InnerLogger()
        {}

        InnerLogger( const std::string &output_file_path )
            :   output_file_path_(output_file_path)
        {
            open_log_file_();
        }

        ~InnerLogger()
        {
            output_.close();
        }

        inline void set_output_file_path( const std::string &output_file_path = "" )
        {
            if (output_file_path.empty())
                output_file_path_ = default_output_file_path;
            else 
                output_file_path_ = output_file_path;
        }

        inline void debug( const std::string &tag, const std::string &message ) const
        {
            log_(eLogLevel::DEBUG, tag, message);
        }

        inline void info( const std::string &tag, const std::string &message ) const
        {
            log_(eLogLevel::INFO, tag, message);
        }

        inline void warning( const std::string &tag, const std::string &message ) const
        {
            log_(eLogLevel::WARNING, tag, message);
        }

        inline void error( const std::string &tag, const std::string &message ) const
        {
            log_(eLogLevel::ERROR_LOG, tag, message);
        }

        inline void fatal( const std::string &tag, const std::string &message ) const
        {
            log_(eLogLevel::FATAL, tag, message);
        }

    private:
        mutable std::string output_file_path_;
        mutable std::ofstream output_;

        static const std::string level_str[5];
        static const std::string default_output_file_path;

        inline void open_log_file_() const
        {
            if (output_file_path_.empty())
                output_file_path_ = default_output_file_path;
            std::string path = cq::dir::app() + output_file_path_;
            output_.open(path, std::ios::app);
            if (!output_.is_open())
                output_.open(cq::dir::app() + default_output_file_path);
        }

        inline std::string get_current_time_str_() const
        {
            SYSTEMTIME sys_time;
            std::stringstream stream;
            GetLocalTime(&sys_time);
            stream << sys_time.wYear << "/" << sys_time.wMonth << "/" << sys_time.wDay << " ";
            stream << std::setfill('0') << std::setw(2) << sys_time.wHour << ":";
            stream << std::setfill('0') << std::setw(2) << sys_time.wMinute << ":";
            stream << std::setfill('0') << std::setw(2) << sys_time.wSecond;
            return stream.str();
        }

        // Example
        // 2020/1/26 10:43:22 [DEBUG] [初始化] QuinBot 初始化完成;

        inline void log_( eLogLevel level, const std::string &tag, const std::string &message ) const
        {
            if (!output_.is_open())
                open_log_file_();
            output_ << get_current_time_str_() << " ";
            output_ << "[" << level_str[(int)level] <<  "] ";
            output_ << "[" << tag << "] ";
            output_ << message << ";\n";
            output_.close();
        }

    };

    const std::string InnerLogger::level_str[5] = { "DEBUG", "INFO", "WARNING", "ERROR", "FATAL" };
    const std::string InnerLogger::default_output_file_path = "quinbot.log";

    class CoolQLogger
    {
        friend class Logger;
    public:
        CoolQLogger() 
        {}

        inline void debug( const std::string &tag, const std::string &message ) const
        {
            cq::logging::debug(tag, message);
        }

        inline void info( const std::string &tag, const std::string &message ) const
        {
            cq::logging::info(tag, message);
        }

        inline void info_success( const std::string &tag, const std::string &message ) const
        {
            cq::logging::info(tag, message);
        }

        inline void warning( const std::string &tag, const std::string &message ) const
        {
            cq::logging::warning(tag, message);
        }

        inline void error( const std::string &tag, const std::string &message ) const
        {
            cq::logging::error(tag, message);
        }

        inline void fatal( const std::string &tag, const std::string &message ) const
        {
            cq::logging::fatal(tag, message);
        }

    private:
        inline void log_( cq::logging::Level level, const std::string &tag, const std::string &message ) const
        {
            cq::logging::log(level, tag, message);
        }
    };

    class Logger
    {
    public:
        Logger()
        {

        }

        inline void set_type( const eLoggerType type )
        {
            type_ = type;
        }

        inline void set_output_file_path( const std::string &path )
        {
            inner_logger_.set_output_file_path(path);
        }

        inline void set_enabled( bool enabled )
        {
            enabled_ = enabled;
        }

        inline void debug( const std::string &tag, const std::string &message ) const
        {
            log_(eLogLevel::DEBUG, tag, message);
        }

        inline void info( const std::string &tag, const std::string &message ) const
        {
            log_(eLogLevel::INFO, tag, message);
        }

        inline void warning( const std::string &tag, const std::string &message ) const
        {
            log_(eLogLevel::WARNING, tag, message);
        }

        inline void error( const std::string &tag, const std::string &message ) const
        {
            log_(eLogLevel::ERROR_LOG, tag, message);
        }

        inline void fatal( const std::string &tag, const std::string &message ) const
        {
            log_(eLogLevel::FATAL, tag, message);
        }

    private:
        eLoggerType type_;
        std::string output_file_path_;
        bool enabled_;

        InnerLogger inner_logger_;
        CoolQLogger coolq_logger_;

        inline cq::logging::Level level_convert_( eLogLevel level ) const
        {
            if (level == eLogLevel::DEBUG) { return cq::logging::DEBUG; }
            if (level == eLogLevel::INFO) { return cq::logging::INFO; }
            if (level == eLogLevel::WARNING) { return cq::logging::WARNING; }
            if (level == eLogLevel::ERROR_LOG) { return cq::logging::ERROR; }
            return cq::logging::FATAL;
        }

        inline void log_( const eLogLevel level, const std::string &tag, const std::string &message ) const
        {
            if (!enabled_) return;
            if (type_ == eLoggerType::INNER) { inner_logger_.log_(level, tag, message); return; }
            if (type_ == eLoggerType::COOLQ) { coolq_logger_.log_(level_convert_(level), tag, message); return; }
            if (type_ == eLoggerType::BOTH) 
            {
                inner_logger_.log_(level, tag, message);
                coolq_logger_.log_(level_convert_(level), tag, message);
                return;
            }
        }
    };
}