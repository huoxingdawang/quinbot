#ifndef __QUINBOT_TASK_QUEUE_H__
#define __QUINBOT_TASK_QUEUE_H__

#include <queue>

#include "command.h"
#include "enums.h"

namespace quinbot
{
    class TaskQueue
    {
    public:
        TaskQueue();

        TaskQueue( const TaskQueue &other );

    private:
    };
}

#endif