#include "file_utils.h"

#include <string>
#include <fstream>
#include <urlmon.h>

#include "../../cqsdk/dir.h"
#include "../../cqsdk/logging.h"

#include "json.hpp"

#include "string_utils.h"

bool is_json_comment( const std::string &str )
{
    std::string::size_type pos = 0;
    while (str[pos] && (str[pos] == ' ' || str[pos] == '\t'))
        ++pos;
    return str[pos] == '/' && str[pos + 1] == '/';
}

namespace quinbot
{
namespace util
{
    void read_json( nlohmann::json &json_out, const std::string &path )
    {
        std::ifstream input;
        input.open(path);
        if (!input.is_open())
            return;
        std::string line;
        std::string json_str;
        while (std::getline(input, line))
        {
            if (line.empty() || is_json_comment(line))
                continue;
            json_str += line;
        }
        json_out = nlohmann::json::parse(json_str);
        input.close();
    }

    long long get_file_size( const std::string &path )
    {
        std::ifstream input;
        std::streamoff size = 0;
        input.open(path, std::ios::binary);
        if (!input.is_open())
            return 0;
        input.seekg(0, std::ios::end);
        size = input.tellg();
        input.close();
        return size;
    }

    long long get_file_size( std::ifstream &fin )
    {
        auto original_pos = fin.tellg();
        std::streamoff size = 0;
        fin.seekg(0, std::ios::end);
        size = fin.tellg();
        fin.seekg(original_pos);
        return size;
    }

    long long read_binary( std::string &data, const std::string &path )
    {
        std::ifstream input;
        input.open(path, std::ios::binary);
        if (!input.is_open())
            return 0;
        long long file_size = get_file_size(input);
        char *p_buf = new char[file_size];
        input.read(p_buf, sizeof(char) * file_size);
        input.close();
        data = std::string(p_buf, file_size);
        delete[] p_buf;
        return file_size;
    }

    void write_binary( const std::string &data, const std::string &path )
    {
        std::ofstream output;
        output.open(path, std::ios::binary | std::ios::trunc);
        if (!output.is_open())
            return;
        output.write(data.c_str(), data.size());
        output.close();
    }

    HRESULT download( const std::string &url, const std::string &save_path )
    {
        HRESULT hresult = URLDownloadToFileW(NULL, string_to_wstring(url).c_str(), string_to_wstring(save_path).c_str(), 0, NULL);
        return hresult;
    }

    std::string get_image_url( const std::string &file_name )
    {
        std::string full_path = cq::dir::root() + "data/image/" + file_name + ".cqimg";
        std::ifstream input;
        input.open(full_path);
        std::string line;
        std::string url;
        while (std::getline(input, line))
        {
            if (str_equals(line, 0, "url="))
            {
                url = std::string(line, 4, line.size() - 4);
                break;
            }
        }
        input.close();
        return url;
    }

    std::string get_image( const std::string &file_name )
    {
        std::string full_path = cq::dir::root() + "data/image/" + file_name;
        std::string url = get_image_url(file_name);
        HRESULT result = download(url, full_path);
        if (result != S_OK)
            cq::logging::error("Get Image", "Fail to get image");
            //throw std::exception("Fail to get image");
        return full_path;
    }

    std::string get_image_with_url( const std::string &file_name, const std::string &url )
    {
        std::string full_path = cq::dir::root() + "data/image/" + file_name;
        HRESULT result = download(url, full_path);
        if (result != S_OK)
            throw std::exception("Fail to get image");
        return full_path;
    }

}
}