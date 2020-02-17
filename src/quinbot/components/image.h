#pragma once

#include <fstream>
#include <string>

#include "../../cqsdk/dir.h"

#include "../utils/file_utils.h"

namespace quinbot
{
namespace component
{
    class Image
    {
    public:
        Image( const std::string &path, bool is_full_path = false )
            :   size_(0)
        {
            if (is_full_path)
            {
                full_path_ = path;
                std::string::size_type pos = full_path_.find("image");
                file_name_ = std::string(full_path_, pos + 6, full_path_.size() - pos - 6);
            }
            else
            {
                full_path_ = cq::dir::root() + "data\\image\\" + path;
                file_name_ = path;
            }
            url_ = util::get_image_url(file_name_);
        }

        std::string path() const
        {
            return full_path_;
        }

        std::string file_name() const
        {
            return file_name_;
        }

        size_t size() const
        {
            return size_;
        }

        std::string data()
        {
            std::string buf;
            size_ = util::read_binary(buf, full_path_);
            return buf;
        }

        operator std::string()
        {
            return data();
        }

        std::string url() const
        {
            return url_;
        }

    private:
        std::string file_name_;
        std::string full_path_;
        std::string url_;
        size_t size_;
    };
}
}