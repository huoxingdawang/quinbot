#pragma once

#include <unordered_map>
#include <string>

#include "../utils/string_utils.h"

namespace quinbot
{
namespace command
{
    class ArgsMap
    {
        
    public:
        std::string error_message;
        ArgsMap() = default;

        inline void add_option( const std::string &option )
        {
            options_.insert({option, true});
        }

        inline void add_arg( const std::string &arg_name, const std::string &arg_value )
        {
            values_.insert({arg_name, arg_value});
        }

        inline void add_va_arg( const std::string &arg_value )
        {
            va_.push_back(arg_value);
        }

        inline void set_error( const std::string &error_message )
        {
            is_error_ = true;
            this->error_message = error_message;
        }

        inline bool get_option( std::string option ) const
        {
            return options_.find(option) != options_.end();
        }

        inline size_t va_size() const
        {
            return va_.size();
        }

        inline bool is_bad() const
        {
            return is_error_;
        }

        template<typename GetterType>
        GetterType get( std::string key, GetterType default_value, bool must ) const
        {
            return _get<GetterType>(key, default_value, must);
        }

        template<typename GetterType>
        GetterType get_va( size_t index, GetterType default_value ) const
        {
            return _va_get<GetterType>(index, default_value);
        }

    private:
        std::unordered_map<std::string, bool> options_;
        std::unordered_map<std::string, std::string> values_;
        std::vector<std::string> va_;
        
        bool is_error_ = false;

        bool util_to_bool_( std::string str ) const
        {
            str = util::ascill_to_lower(str);
            if (str == "true" || str == "1")
                return true;
            else if (str == "false" || str == "0")
                return false;
        }

#define GETTER_DEF( GetterType, Converter ) \
template<> \
 ##GetterType _get<##GetterType##>(std::string main_key, ##GetterType default_value, bool must) const \
{ \
    if (values_.find(main_key) != values_.end()) \
        return (##Converter##(values_.at(main_key))); \
    if (must) \
        throw std::underflow_error("有限参数过少"); \
    return default_value; \
}

        template<typename GetterType>
        GetterType _get( std::string main_key, GetterType default_value, bool must ) const
        {
            if (values_.find(main_key) != values_.end())
                return static_cast<GetterType>(values_.at(main_key));
            if (must)
                throw std::underflow_error("有效参数过少");
            return default_value;
        }

        GETTER_DEF(int32_t, std::stoi);
        GETTER_DEF(int64_t, std::stoll);
        GETTER_DEF(double, std::stod);
        GETTER_DEF(float, std::stof);
        GETTER_DEF(std::string);
        GETTER_DEF(bool, util_to_bool_);

#undef GETTER_DEF

#define VA_GETTER_DEF( GetterType, Converter ) \
template<> \
 ##GetterType _va_get<##GetterType##>(size_t idx, ##GetterType default_value) const \
{ \
    if (idx >= va_.size()) \
        return default_value; \
    return (##Converter(va_[idx])); \
}
    template<typename GetterType>
    GetterType _va_get( size_t idx, GetterType default_value ) const
    {
        if (idx >= va_.size())
            return default_value;
        return static_cast<GetterType>(va_[idx]);
    }

    VA_GETTER_DEF(int32_t, std::stoi);
    VA_GETTER_DEF(int64_t, std::stoll);
    VA_GETTER_DEF(double, std::stod);
    VA_GETTER_DEF(float, std::stof);
    VA_GETTER_DEF(std::string);
    VA_GETTER_DEF(bool, util_to_bool_);

#undef VA_GETTER_DEF
    };
}
}