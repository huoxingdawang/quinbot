#pragma once

#include <fstream>
#include <string>

#include "json.hpp"

static bool is_json_comment( const std::string &str );

namespace quinbot
{
namespace util
{
    void read_json( nlohmann::json &json_out, const std::string &path );

    int64_t get_file_size( const std::string &path );
    int64_t get_file_size( std::ifstream &fin );

    int64_t read_binary( std::string &data_out, const std::string &path );
    void write_binary( const std::string &data, const std::string &path );

    HRESULT download( const std::string &url, const std::string &save_path );

    std::string get_image_url( const std::string &file_name );
    std::string get_image( const std::string &file_name );
}
}