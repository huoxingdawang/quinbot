#include "cqsdk/cqsdk.h"

#include "quinbot/quinbot.h"

CQ_MAIN
{
    cq::app::on_enable = []
    {
    };

    cq::event::on_group_msg = [](const cq::GroupMessageEvent &e)
    {
        
    };

    cq::event::on_private_msg = [](const cq::PrivateMessageEvent &e)
    {
        
    };
}

