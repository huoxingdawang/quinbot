#pragma once

#include "../../plugin.h"
#include "../../bot.h"

#include "../../3rdparty/croncpp.h"

void next_timer_( int32_t hour )
{
    auto &s = quinbot::bot.get_scheduler();
    for (auto id : quinbot::bot.get_config().groups)
    {
        if (id != 712548068)
            continue;
        cq::api::send_group_msg(id, "[CQ:image,file=ED82F8BA97BD1B28DBE39EBC48060CFB.jpg]");
    }
    ++hour;
    if (hour == 24)
        hour = 0;
    std::string hour_str = std::to_string(hour);
    if (hour_str.size() == 1)   
        hour_str = "0" + hour_str;
    hour_str += ":00:00";
    cq::logging::debug("time_str2", hour_str);
    s.at(hour_str, next_timer_, hour);
}

namespace quinbot
{
namespace plugin
{
    class SetTimerCommand final : public command::GroupCommand
    {
        using eExecuteResult = command::eExecuteResult;
    public:
        SetTimerCommand()
            :   command::GroupCommand("set_timer")
        {

        }

        eExecuteResult process( const command::CommandInfo &info ) override
        {

        }

    private:

    };

    class TimerPlugin final : public Plugin
    {
    public:
        TimerPlugin()
            :   Plugin("timer")
        {
        }

        void on_enable() override
        {
            int32_t hour = util::date_today("hour");
            ++hour;
            if (hour == 24)
                hour = 0;
            std::string current_hour = std::to_string(hour);
            if (current_hour.size() == 1)
                current_hour = "0" + current_hour;
            current_hour += ":00:00";
            cq::logging::debug("time_str", current_hour);
            auto &s = bot.get_scheduler();
            s.at(current_hour, next_timer_, hour);

            
        }
        
    private:
        
    };
}
}