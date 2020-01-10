#ifndef __QUINBOT_ENUMS__
#define __QUINBOT_ENUMS__

namespace quinbot
{
    enum class eExcuteResult
    {
        SUCCESS,
        TOO_FEW_PARAMETER,
        TOO_MANY_PARAMETER,
        TOO_MANY_PARAMETER_BUT_OMIT,
    };

    enum class eDatabaseResult
    {
        SUCCESS

    };
}

#endif