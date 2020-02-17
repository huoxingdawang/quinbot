#ifndef __QUINBOT_ENUMS__
#define __QUINBOT_ENUMS__

namespace quinbot::command
{
    enum class eCommandType
    {
        UNKNOWN,
        PUBLIC,
        PRIVATE,
        GROUP,
        DISCUSS
    };

    enum class eCommandArgType
    {
        TEXT,
        EMOJI,
        FACE,
        IMAGE,
        RECORD,
        AT,
        RPS,
        DICE,
        SHAKE,
        ANNOYMOUS,
        SHARE,
        CONTACT,
        LOCATION,
        MUSIC
    };

    enum class eExecuteResult
    {
        UNKNOWN,
        NOT_A_COMMAND,
        SUCCESS,
        TOO_FEW_PARAMETER,
        TOO_MANY_PARAMETER,
        TOO_MANY_PARAMETER_BUT_OMIT,
        TYPE_ERROR,
        USER_ERROR,
        NO_CALLBACK
    };

    enum class eParseResult
    {
        SUCCESS,
        NOT_A_COMMAND
    };

    enum class eCommandSpliter
    {
        INNER,
        CUSTOM
    };
}
    
namespace quinbot
{
    enum class eMessageType
    {
        PRIVATE,
        GROUP,
        DISCUSS
    };

    enum class eLoggerType
    {
        INNER,
        COOLQ,
        BOTH
    };

    enum class eLogLevel
    {
        DEBUG,
        INFO,
        WARNING,
        ERROR_LOG,
        FATAL
    };

    enum class eDatabaseResult
    {
        SUCCESS

    };

    enum class eSqlite_DataType
    {
        NULLVALUE,

        INTEGER,
        SMALLINT,

        REAL,
        FLOAT,
        DOUBLE,

        TEXT,
        CHAR1,
        CHAR2,
        CHAR16,
        CHAR255,
        VARCHAR16,
        VARCHAR255,

        BLOB,

        DATE,
        TIME,
        TIMESTAMP
    };

    enum class eSqlite_Constraint
    {
        PRIMARY_KEY,
        FOREIGIN_KEY,
        NOT_NULL,
        DEFAULT,
        UNIQUE,
        CHECK
    };

    enum class eUserType
    {
        ADMINISTRATOR,
        CUSTOMER
    };
}

#endif