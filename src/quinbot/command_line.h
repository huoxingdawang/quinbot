#ifndef __COMMAND_LINE_H__
#define __COMMAND_LINE_H__

#include <initializer_list>
#include <string>
#include <vector>

#include <stdint.h>

namespace quinbot
{
    class CommandLine
    {
        using ParameterListType = std::vector<std::string>;
    public:
        CommandLine( const std::string &name, const ParameterListType &list )
            :   name_(name),
                list_(list)
        {
        }

        CommandLine( const std::string &name, std::initializer_list<std::string> il )
            :   name_(name)
        {
            for (const auto &str : il)
            {
                list_.push_back(str);
            }
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

        const std::string &operator[]( size_t index ) const
        {
            return list_[index];
        }

        inline std::string name() const
        {
            return name_;
        }

        inline int32_t size() const
        {
            return list_.size();
        }

        inline const ParameterListType &list() const
        {
            return list_;
        }

        inline void set_name( const std::string &new_name )
        {
            name_ = new_name;
        }

        inline void set_parameter_list( const ParameterListType &new_list )
        {
            list_ = new_list;
        }

        inline void add_parameter( const std::string parameter_name )
        {
            list_.push_back(parameter_name);
        }

        inline void add_parameter( const char *parameter_name )
        {
            list_.emplace_back(parameter_name);
        }

        inline void pop_parameter()
        {
            list_.pop_back();
        }

        inline void clear()
        {
            list_.clear();
        }

        bool is_number( size_t index, bool strict = false ) const
        {
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
                    || (cur_str[i] >= '0' && cur_str[i] <= 9);
        }

        inline bool is_boolean( size_t index ) const
        {
            return list_[index] == "true" || list_[index] == "false";
        }

        inline int32_t as_int32( size_t index ) const
        {
            return std::stoi(list_[index]);
        }

        inline int64_t as_int64( size_t index ) const
        {
            return std::stoll(list_[index]);
        }

        inline float as_float( size_t index ) const
        {
            return std::stof(list_[index]);
        }

        inline double as_double( size_t index ) const
        {
            return std::stod(list_[index]);
        }

        inline long double as_long_double( size_t index ) const
        {
            return std::stold(list_[index]);
        }

        inline bool as_boolean( size_t index ) const
        {
            if (list_[index] == "false" || list_[index] == "0")
                return false;
            return true;
        }

        inline std::string as_string( size_t index ) const
        {
            return list_[index];
        }



    private:
        std::string name_;
        ParameterListType list_;
    };
}

#endif