#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <tuple>

#include <cpr/cpr.h>
#include <Python.h>

#include "../../plugin.h"
#include "../../bot.h"

namespace quinbot
{
namespace plugin
{
    class Ascii2dSearchCommand final : public command::PublicCommand
    {
        struct ArtInfo
        {
            std::string local_file_name;
            std::string art_name;
            std::string art_link;
            std::string artist_name;
            std::string artist_link;
        };

    public:
        Ascii2dSearchCommand()
            :   command::PublicCommand("ascii2d")
        {
            this->set_aliases({"查找图片", "查询出处"});
            this->set_help_message(
                "格式: ascii2d [可选 搜索方式] [图片/图片URL]\n"
                "参数[搜索方式]可选值\n" 
                "[1] color/颜色/颜色搜索 - 以颜色为依据查找\n"
                "[2] bovw/特征/特征搜索 - 以特征为依据查找\n"
                "参数[图片/图片URL]");
        }

        ~Ascii2dSearchCommand() = default;

        command::eExecuteResult process( const command::CommandInfo &info ) override
        {
            auto &cl = info.command_line;
            ArtInfo art;

            if (cl.size() == 1)
            {
                if (cl.type_of(0) != command::eCommandArgType::IMAGE)
                {
                    info.send_back(format_error_(u8"参数错误", u8"参数[1]应当为图片"));
                    return command::eExecuteResult::TYPE_ERROR;
                }
                auto img = cl.as_image(0);
                art = color_search_(img);
                //art = search_image(0, img.url());
            }
            else if (cl.size() == 2)
            {
                if (cl[0] == u8"颜色" || cl[0] == u8"颜色搜索" || cl[0] == u8"color")
                {
                    if (cl.type_of(1) != command::eCommandArgType::IMAGE)
                    {
                        info.send_back(format_error_(u8"参数错误", u8"参数[2]应当为图片"));
                        return command::eExecuteResult::TYPE_ERROR;
                    }
                    auto img = cl.as_image(1);
                    art = color_search_(img);
                    //art = search_image(0, img.url());
                }
                else if (cl[0] == u8"特征" || cl[0] == u8"特征搜索" || cl[0] == u8"bovw")
                {
                    if (cl.type_of(1) != command::eCommandArgType::IMAGE)
                    {
                        info.send_back(format_error_(u8"参数错误", u8"参数[2]应当为图片"));
                        return command::eExecuteResult::TYPE_ERROR;
                    }
                    auto img = cl.as_image(1);
                    art = bovw_search_(img);
                    //art = search_image(1, img.url());
                }
                else
                {
                    info.send_back(format_error_(u8"参数错误", u8"无法识别的查找选项 \"" + cl[0] + "\""));
                    return command::eExecuteResult::USER_ERROR;
                }
            }
            
            info.send_back(format_result_(art));
            return command::eExecuteResult::SUCCESS;
        }

        ArtInfo search_image( int32_t type, const std::string &url ) const
        {
            auto &logger = quinbot::bot.get_logger();
            Py_Initialize();

            PyRun_SimpleString("import sys");
            PyRun_SimpleString("sys.path.append('E:/quinbot/quinbot/src/quinbot/plugins/ascii2d_search')");
            PyObject *p_module = PyImport_ImportModule("ascii2d_search");

            if (!p_module)
                logger.error("Python", "Python加载失败");

            PyObject *p_process_func = PyObject_GetAttrString(p_module, "ascii2d_search");
            PyObject *p_search_type = PyLong_FromLong(type);
            PyObject *p_search_url = PyUnicode_FromString(url.c_str());
            PyObject *p_args = PyTuple_New(2);
            PyTuple_SetItem(p_args, 0, p_search_type);
            PyTuple_SetItem(p_args, 1, p_search_url);
            PyObject *p_ret = PyObject_CallObject(p_process_func, p_args);
            
            int32_t failed = util::pydict_as_int32(p_ret, "failed");
            if (failed == 1)
            {
                logger.error("ascii2d", "搜索图片失败");
            }

            ArtInfo info;
            info.art_name = util::pydict_as_string(p_ret, "art_name");
            info.art_link = util::pydict_as_string(p_ret, "art_link");
            info.artist_name = util::pydict_as_string(p_ret, "artist_name");
            info.artist_link = util::pydict_as_string(p_ret, "artist_link");
            std::string preview_link = util::pydict_as_string(p_ret, "preview_link");

            logger.debug("LINK", preview_link);

            size_t start_pos = preview_link.rfind("/") + 1;
            info.local_file_name = "temp/" + std::string(preview_link, start_pos, preview_link.size() - start_pos);
            
            /*auto download_result = util::download(preview_link, cq::dir::root() + "data/image/" + info.local_file_name);
            if (download_result != S_OK)
                logger.error("下载失败", "ascii2d 无法下载预览图");*/
            Py_Finalize();
            return info;
        }

    private:
        inline std::string format_result_( const ArtInfo &art ) const
        {
            cq::message::Message msg;
            
            msg += cq::message::MessageSegment::image(art.local_file_name);
            msg += cq::message::MessageSegment::text(u8"「作者: " + art.artist_name + u8"」 " + art.artist_link + "\n");
            msg += cq::message::MessageSegment::text(u8"「作品名: " + art.art_name + u8"」 " + art.art_link);
            return std::to_string(msg);
        }

        inline std::string format_error_( const std::string &error_type, const std::string &message ) const
        {
            std::string fstr = u8"[ascii2d] " + error_type + ": " + message;
            return fstr;
        }

        inline std::string get_color_search_url_( const std::string &img_url ) const
        {
            std::string raw_url = "https://ascii2d.net/search/url/" + img_url;
            cpr::Response r = cpr::Get(cpr::Url(raw_url));
            return r.url;
        }

        inline std::string get_bovw_search_url_( const std::string &img_url, bool from_color_search_url = false ) const
        {
            std::string result;
            if (from_color_search_url)
            {
                result = img_url;
            }
            else
            {
                std::string raw_url = "https://ascii2d.net/search/url/" + img_url;
                cpr::Response r = cpr::Get(cpr::Url(raw_url));
                result = r.url;
            }
            size_t start_pos = result.find("color");
            result.replace(start_pos, 5, "bovw");
            return result;
        }

        std::pair<std::string, std::string> get_preview_image_detail_( const std::string &raw_str ) const
        {
            size_t pos1 = 11;
            size_t pos2 = raw_str.find("\"", pos1);
            std::string relative_path = std::string(raw_str, pos1, pos2 - pos1);

            size_t start_pos = relative_path.rfind("/") + 1;
            std::string file_name = std::string(relative_path, start_pos, relative_path.size() - start_pos);
            
            std::string img_url = "https://ascii2d.net/" + relative_path;
            std::string local_file_name = "temp/" + file_name;

            cq::logging::debug("get_preview", "url = " + img_url);
            cq::logging::debug("get_preview", "local = " + local_file_name);

            return {img_url, local_file_name};
        }

        std::pair<std::string, std::string> get_art_detail_( const std::string &line ) const
        {
            size_t pos1 = line.find("rel=\"noopener\"") + 21;
            size_t pos2 = line.find("\"", pos1);
            size_t pos3 = line.rfind("</a>");

            // [link, name]
            std::string link = std::string(line, pos1, pos2 - pos1);
            std::string name = std::string(line, pos2 + 2, pos3 - pos2 - 2);
            cq::logging::debug("get_detail", "link = " + link);
            cq::logging::debug("get_detail", "name = " + name);
            return {link, name};
        }

        std::pair<std::string, std::string> find_preview_image_( const std::string &text, std::stringstream &buf, int64_t &current_find_pos ) const
        {
            int64_t item_box_pos = static_cast<int64_t>(text.find("<div class='row item-box'>", current_find_pos));
            current_find_pos = item_box_pos;
            buf.seekg(current_find_pos);
            std::string line;
            while (std::getline(buf, line))
            {
                if (util::starts_with(line, "<img"))
                    return get_preview_image_detail_(line);
            }
            return {"", ""};
        }

        bool find_art_detail_( ArtInfo &info, const std::string &text, std::stringstream &buf, int64_t &current_find_pos ) const
        {
            int64_t detail_box_pos = static_cast<int64_t>(text.find("<div class='detail-box gray-link'>", current_find_pos));
            current_find_pos = detail_box_pos;
            buf.seekg(current_find_pos);

            int32_t record_flag = 0;

            ArtInfo result;

            std::string line;
            while (std::getline(buf, line))
            {
                if (line == "</div>")
                    break;
                if (util::starts_with(line, "<a"))
                {
                    if (record_flag == 0)
                    {
                        auto [link, name] = get_art_detail_(line);
                        result.art_link = link;
                        result.art_name = name;
                        record_flag = 1;
                    }
                    else if (record_flag == 1)
                    {
                        auto [link, name] = get_art_detail_(line);
                        result.artist_link = link;
                        result.artist_name = name;
                        break;
                    }
                }
            }
            if (record_flag != 1)
                return false;
            info = result;
            return true;
        }

        ArtInfo search_( const component::Image &img, const std::string &url ) const
        {   
            cpr::Response r = cpr::Get(cpr::Url(url));

            int record_flag = 0;
            bool image_record_flag = false;

            std::stringstream buf(r.text);
            int64_t current_find_pos = 0;

            ArtInfo result;

            while (buf)
            {
                auto [img_url, local_file_name] = find_preview_image_(r.text, buf, current_find_pos);
                if (find_art_detail_(result, r.text, buf, current_find_pos))
                {
                    auto download_result = util::download(img_url, cq::dir::root() + "data/image/" + local_file_name);
                    if (download_result != S_OK)
                        bot.get_logger().error("Download Error", "Fail to download preview image");
                    
                    result.local_file_name = local_file_name;
                    break;
                }
            }

            cq::logging::debug("local_file_name", result.local_file_name);
            cq::logging::debug("art_link", result.art_link);
            cq::logging::debug("art_name", result.art_name);
            cq::logging::debug("artist_link", result.artist_link);
            cq::logging::debug("artist_name", result.artist_name);

            return result;
        }

        inline ArtInfo color_search_( const component::Image &img ) const
        {
            return search_(img, get_color_search_url_(img.url()));
        }

        inline ArtInfo bovw_search_( const component::Image &img ) const
        {
            return search_(img, get_bovw_search_url_(img.url()));
        }

    };

    class Ascii2dSearchPlugin final : public Plugin
    {
    public:
        Ascii2dSearchPlugin()
            :   Plugin("ascii2d_search")
        {}

        void on_enable() override
        {
            auto &manager = bot.get_command_manager();
            manager->register_command<Ascii2dSearchCommand>();
        }

    private:
    };
}
}

