#pragma once

#include <string>
#include <fstream>
#include <memory>
#include <mutex>

#include "utils/cq_util.h"
#include "utils/json.hpp"
#include "utils/file_utils.h"
#include "utils/python_utils.h"
#include "utils/math_utils.h"
#include "utils/useful.h"

#include "command/arg_map.h"
#include "command/arg_parser.h"
#include "command/builtin_command.h"
#include "command/command_line.h"
#include "command/command_manager.h"
#include "command/command_parser.h"
#include "command/command_processer.h"
#include "command/command.h"
#include "config.h"
#include "enums.h"
#include "logger.h"
#include "plugin.h"
#include "plugin_manager.h"

#include "3rdparty/scheduler/Scheduler.h"

#define INITBOT( Component ) init_##Component##_()
#define INITBOT_BEFORE( Component ) init_##Component##_before_()
#define INITBOT_AFTER( Component ) init_##Component##_after_()

namespace quinbot
{
    class BotInstance
    {
        using json = nlohmann::json;
        using CommandManager = command::CommandManager;
        using CommandParser = command::CommandParser;
        using CommandProcesser = command::CommandProcesser;
        using PluginManager = plugin::PluginManager;
    public:
        BotInstance()
        {
        }

        inline void on_initialize()
        {
            if (!p_plugin_manager_)
                INITBOT_BEFORE(plugin_manager);
        }

        inline void on_coolq_start()
        {
        }

        inline void on_coolq_exit()
        {
        }

        inline void on_enable()
        {
            INITBOT(python);
            INITBOT(config);
            INITBOT(logger);
            INITBOT(scheduler);
            INITBOT(command_manager);
            INITBOT(command_parser);
            INITBOT(command_processer);
            INITBOT(plugin_manager);
        }

        inline void on_disable()
        {    
            PyGILState_Ensure();
	        Py_Finalize();
        }

        inline void set_config( const Config &config )
        {
            config_ = config;
        }

        inline Config &get_config()
        {
            return config_;
        }

        inline Logger &get_logger()
        {
            return logger_;
        }

        inline std::shared_ptr<CommandManager> get_command_manager()
        {
            return p_command_manager_;
        }

        inline std::shared_ptr<CommandParser> get_command_parser()
        {
            return p_command_parser_;
        }

        inline std::shared_ptr<CommandProcesser> get_command_processer()
        {
            return p_command_processer_;
        }

        inline std::shared_ptr<PluginManager> get_plugin_manager()
        {
            if (!p_plugin_manager_)
                INITBOT_BEFORE(plugin_manager);
            return p_plugin_manager_;
        }

        inline PyObject *get_python_module( const std::string &name )
        {
            if (python_modules_.find(name) == python_modules_.end())
                throw std::exception("fail to get python module");
            return python_modules_.at(name);
        }

        inline std::map<std::string, PyObject *> &get_python_modules()
        {
            return python_modules_;
        }

        inline Bosma::Scheduler &get_scheduler()
        {
            return *p_scheduler_;
        }

        template<typename EventType>
        inline void command_process( const EventType &event )
        {
            p_command_processer_->run(event);
        }

    private:
        Config config_;
        Logger logger_;
        std::shared_ptr<CommandManager> p_command_manager_;
        std::shared_ptr<CommandParser> p_command_parser_;
        std::shared_ptr<CommandProcesser> p_command_processer_;
        std::shared_ptr<PluginManager> p_plugin_manager_;
        std::map<std::string, PyObject *> python_modules_;
        std::shared_ptr<Bosma::Scheduler> p_scheduler_;

        inline void init_python_()
        {
            Py_Initialize();
	        PyEval_InitThreads();

            PyRun_SimpleString("import sys");
            PyRun_SimpleString("sys.path.append('E:/quinbot/quinbot/src/quinbot/plugins/random_illustration')");
            //PyRun_SimpleString("sys.path.append('E:/quinbot/quinbot/src/quinbot/plugins/pixiv')");
            python_modules_.insert({"random_illustration", PyImport_ImportModule("random_illustration")});
            //python_modules_.insert({"pixiv", PyImport_ImportModule("pixiv")});
            
            if (PyEval_ThreadsInitialized())
            {
				PyEval_SaveThread();
                cq::logging::info("Python", "Python加载多线程成功");
            }
            else
            {
                cq::logging::error("Python", "Python加载多线程失败");
                return;
            }
        }

        inline void init_config_()
        {
            json json_config;
            util::read_json(json_config, cq::dir::app() + "config.json");
            config_.from_json(json_config);
        }

        inline void init_plugin_manager_before_()
        {
            p_plugin_manager_ = std::make_shared<PluginManager>();
        }

        inline void init_plugin_manager_()
        {
            logger_.info(u8"初始化", u8"插件系统初始化完成: 已成功加载 " + std::to_string(p_plugin_manager_->size()) + " 个插件");
        }

        inline void init_logger_()
        {
            logger_.set_enabled(config_.enable_logger);
            logger_.set_type(config_.logger_type);
            logger_.set_output_file_path(config_.logger_output_file_path);
            logger_.info(u8"初始化", u8"日志系统初始化完成");
        }

        inline void init_command_manager_()
        {
            p_command_manager_ = std::make_shared<CommandManager>();
            p_command_manager_->set_prefixes(config_.command_prefixes);
            p_command_manager_->set_allow_aliases(config_.allow_aliases);
            p_command_manager_->set_case_sensitive(config_.case_sensitive);
            command::builtin::register_all_builtin_commands(p_command_manager_);
            logger_.info(u8"初始化", u8"命令管理器初始化完成");
            logger_.info(u8"size", std::to_string(p_command_manager_->size()));
        }

        inline void init_command_parser_()
        {
            if (config_.command_spliter == command::eCommandSpliter::INNER)
                p_command_parser_ = std::make_shared<CommandParser>(config_.command_delimiter, config_.command_arg_range_left, config_.command_arg_range_right);
            else if (config_.command_spliter == command::eCommandSpliter::CUSTOM)
                p_command_parser_ = std::make_shared<CommandParser>(config_.command_split_regex);
            p_command_parser_->set_stripped(config_.stripped);
            p_command_parser_->set_strip_chars(config_.strip_chars);
            logger_.info(u8"初始化", u8"命令解释器初始化完成");
        }

        inline void init_command_processer_()
        {
            p_command_processer_ = std::make_shared<CommandProcesser>(p_command_manager_, p_command_parser_);
            logger_.info(u8"初始化", u8"命令执行器初始化完成");
        }

        inline void init_scheduler_()
        {
            p_scheduler_ = std::make_shared<Bosma::Scheduler>(12);
            logger_.info("初始化", "定时任务器初始化完成");
        }
    };

    BotInstance bot;

}

#undef INITBOT
#undef INITBOT_BEFORE
#undef INITBOT_AFTER