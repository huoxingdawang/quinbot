#pragma once

#include <initializer_list>
#include <string>
#include <vector>
#include <array>

#include <stdint.h>

#include "../logger.h"
#include "../components/image.h"

#define _QUINBOT_DEBUG 1

#if defined(_QUINBOT_DEBUG)
#   define CHECK_RANGE( index ) {if (is_out_of_range_(index)) { throw std::out_of_range("Out of range"); }}
#else
#   define CHECK_RANGE( index )
#endif

namespace quinbot
{
namespace command
{
    class CommandLine
    {
        using ArgList = std::vector<std::string>;
    public:
        CommandLine() = default;

        CommandLine( const std::string &name, const ArgList &list )
            :   name_(name),
                list_(list)
        {
        }

        CommandLine( const std::string &name, std::initializer_list<std::string> args )
            :   name_(name)
        {
            for (const auto &arg : args)
                list_.push_back(arg);
        }

        CommandLine( const CommandLine &other )
            :   name_(other.name_),
                list_(other.list_)
        {
        }

        CommandLine &operator=(const CommandLine &other )
        {
            name_ = other.name_;
            list_ = other.list_;
            return *this;
        }

        inline std::string name() const
        {
            return name_;
        }

        inline size_t size() const
        {
            return list_.size();
        }

        inline const ArgList &list() const
        {
            return list_;
        }

        inline ArgList &list()
        {
            return list_;
        }

        inline std::string get_str( size_t index )
        {
            CHECK_RANGE(index);
            return list_[index];
        }

        const std::string &operator[]( size_t index ) const
        {
            CHECK_RANGE(index);
            return list_[index];
        }

        std::string &operator[]( size_t index )
        {
            CHECK_RANGE(index);
            return list_[index];
        }

        auto begin()        {   return list_.begin();   }
        auto begin() const  {   return list_.begin();   }
        auto end()          {   return list_.end();     }
        auto end() const    {   return list_.end();     }

        inline void set_name( const std::string &new_name )
        {
            name_ = new_name;
        }

        inline void set_args_list( const ArgList &new_list )
        {
            list_ = new_list;
        }

        inline void add_arg( const std::string arg )
        {
            list_.push_back(arg);
        }

        inline void pop_arg()
        {
            list_.pop_back();
        }

        inline void clear()
        {
            list_.clear();
        }

        bool is_number( size_t index, bool strict = false ) const
        {
            CHECK_RANGE(index);
            const std::string &cur_str = list_[index];
            size_t i = 0;
            while (cur_str[i] == ' ')
                ++i;
            if (strict)
            {
                int32_t point_count = 0;
                i = cur_str[i] == '-' ? i + 1 : i;
                for (; cur_str[i]; ++i)
                {
                    if (cur_str[i] == '.')
                        ++point_count;
                    if (cur_str[i] < '0' || cur_str[i] > '9' || point_count > 2)
                        return false;
                }
                return true;
            }
            return  (cur_str[i] == '-' && cur_str[i + 1] >= '0' && cur_str[i + 1] <= '9')
                    || (cur_str[i] >= '0' && cur_str[i] <= '9');
        }

        inline bool is_boolean( size_t index ) const
        {
            CHECK_RANGE(index);
            return list_[index] == "true" || list_[index] == "false";
        }

        inline bool is_image( size_t index ) const
        {
            return list_[index].compare(0, 9, "[CQ:image");
        }

        inline bool is_record( size_t index ) const
        {
            return list_[index].compare(0, 10, "[CQ:record");
        }

        inline int32_t as_int32( size_t index ) const
        {
            CHECK_RANGE(index);
            return std::stoi(list_[index]);
        }

        inline int64_t as_int64( size_t index ) const
        {
            CHECK_RANGE(index);
            return std::stoll(list_[index]);
        }

        inline float as_float( size_t index ) const
        {
            CHECK_RANGE(index);
            return std::stof(list_[index]);
        }

        inline double as_double( size_t index ) const
        {
            CHECK_RANGE(index);
            return std::stod(list_[index]);
        }

        inline long double as_long_double( size_t index ) const
        {
            CHECK_RANGE(index);
            return std::stold(list_[index]);
        }

        inline bool as_boolean( size_t index ) const
        {
            CHECK_RANGE(index);
            if (list_[index] == "false" || list_[index] == "0")
                return false;
            return true;
        }

        inline std::string as_string( size_t index ) const
        {
            CHECK_RANGE(index);
            return list_[index];
        }

        inline component::Image as_image( size_t index ) const
        {
            CHECK_RANGE(index);
            size_t pos1 = list_[index].find("file") + 5;
            size_t pos2 = list_[index].rfind("]");
            return component::Image(util::get_image(std::string(list_[index], pos1, pos2 - pos1)), true);
        }

        inline std::string as_record( size_t index ) const
        {
            CHECK_RANGE(index);
            /*size_t pos1 = list_[index].find("file") + 5;
            size_t pos2 = pos1;
            while (list_[index][pos2] != ']' && list_[index][pos2] != ',')
                ++pos2;
            */
           std::string local_name = cq::api::get_record(list_[index], "mp4");
        }

        inline eCommandArgType type_of( size_t index ) const
        {
            CHECK_RANGE(index);
            std::string cq_code = get_cq_code_(index);
            if (cq_code.empty() || type_matcher_.find(cq_code) == type_matcher_.end())
                return eCommandArgType::TEXT;
            return type_matcher_.at(cq_code);
        }

        inline bool try_as_int32( size_t index, int32_t &out ) const
        {
            static const int32_t default_value = 0;
            if (is_number(index))
            {
                out = as_int32(index);
                return true;
            }
            out = default_value;
            return false;
        }

        inline int64_t try_as_int64( size_t index, int64_t &out ) const
        {
            static const int64_t default_value = 0LL;
            if (is_number(index))
            {
                out = as_int64(index);
                return true;
            }
            out = default_value;
            return false;
        }

        inline float try_as_float( size_t index ) const
        {
            static const float default_value = 0.0f;
            return is_number(index) ? as_float(index) : default_value;
        }

        inline double try_as_double( size_t index ) const
        {
            static const double default_value = 0.0;
            return is_number(index) ? as_double(index) : default_value;
        }

    private:
        std::string prefix_;
        std::string name_;
        ArgList list_;

        static std::unordered_map<std::string, eCommandArgType> type_matcher_;

        inline bool is_out_of_range_( size_t index ) const
        {
            return index < 0 || (index > size());
        }

        std::string get_cq_code_( size_t index ) const
        {
            CHECK_RANGE(index);
            const std::string &str = list_[index];
            size_t beg = str.find("[CQ:");
            size_t end = 0;
            if (beg == std::string::npos)
                return "";
            beg += 4;
            end = beg;
            while (str[end] && str[end] != ']' && str[end] != ',')
                ++end;
            return std::string(str, beg, end - beg);
        }
    };

    std::unordered_map<std::string, eCommandArgType> CommandLine::type_matcher_ = {
        {"emoji", eCommandArgType::EMOJI},
        {"face", eCommandArgType::FACE},
        {"image", eCommandArgType::IMAGE},
        {"record", eCommandArgType::RECORD},
        {"at", eCommandArgType::AT},
        {"rps", eCommandArgType::RPS},
        {"dice", eCommandArgType::DICE},
        {"shake", eCommandArgType::SHAKE},
        {"annoymout", eCommandArgType::ANNOYMOUS},
        {"share", eCommandArgType::SHARE},
        {"contact", eCommandArgType::CONTACT},
        {"location", eCommandArgType::LOCATION},
        {"music", eCommandArgType::MUSIC}
        
    };
}
}
