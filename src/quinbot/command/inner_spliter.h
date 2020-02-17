#pragma once

#include <memory>
#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>

#include "command_line.h"
#include "command_manager.h"
#include "../enums.h"
#include "../utils/string_utils.h"

static std::unordered_map<char, char> unescape_map_ = {
    {'t', '\t'},
    {'\"', '\"'},
    {'n', '\n'},
    {'\\', '\\'}
};

static inline char unescape_( char to_unescape )
{
    return unescape_map_[to_unescape];
}

static inline void outbuf_( std::stringstream &stream, std::vector<std::string> &out )
{
    auto str = stream.str();
    if (str.empty())
        return;
    out.push_back(stream.str());
    stream.str("");
}

namespace quinbot
{
namespace command
{
    void inner_split( const std::string &str, std::vector<std::string> &result, const std::string &delimiter, const std::string &range_left, const std::string &range_right, bool all_coolq_code = false )
    {
        typedef int State;
        static const State TEXT = 0;
        static const State CQCODE = 1;
        static const State DELIMITER = 2;
        static const State RANGE_TEXT = 3;
        
        auto it = str.cbegin();
        auto end = str.cend();
        State state = TEXT;
        std::stringstream buf;

        result.clear();

        for (; it != end; ++it)
        {
            char cur = *it;
            switch (state)
            {
            case TEXT:
            {
                if (cur == '\\' && unescape_map_.find(*(it + 1)) != unescape_map_.end())
                {
                    buf << unescape_(*(it + 1));
                    ++it;
                }
                else if (util::str_equals(str, it, "[CQ:") )
                {
                    if (!all_coolq_code && util::str_equals(str, it + 4, "face"))
                    {
                        buf << "[CQ:face";
                        it += 7;
                    }
                    else if (!all_coolq_code && util::str_equals(str, it + 4, "at"))
                    {
                        buf << "[CQ:at";
                        it += 5;
                    }
                    else
                    {
                        state = CQCODE;
                        outbuf_(buf, result);
                        buf << "[CQ:";
                        it += 3;
                    }
                }
                else if (util::str_equals(str, it, delimiter))
                {
                    state = DELIMITER;
                    it += delimiter.size() - 1;
                }
                else if (it == end - 1)
                {
                    buf << cur;
                    outbuf_(buf, result);
                }
                else if (util::str_equals(str, it, range_left))
                {
                    state = RANGE_TEXT;
                    buf << range_left;
                    it += range_left.size() - 1;
                }
                else
                {
                    buf << cur;
                }
                break;
            }
            case RANGE_TEXT:
            {
                if (cur == '\\' && unescape_map_.find(*(it + 1)) != unescape_map_.end())
                {
                    buf << unescape_(*(it + 1));
                    ++it;
                }
                else if (util::str_equals(str, it, range_right))
                {
                    state = TEXT;
                    buf << range_right;
                    it += range_right.size() - 1;
                }
                else
                {
                    buf << cur;
                }
                break;
            }
            case DELIMITER:
            {
                if (!util::str_equals(str, it, delimiter))
                {
                    outbuf_(buf, result);
                    --it;
                    state = TEXT;
                }
                else
                {
                    it += delimiter.size() - 1;
                }
                break;
            }
            case CQCODE:
            {
                if (cur == ']')
                {
                    state = TEXT;
                    buf << ']';
                    outbuf_(buf, result);
                }
                else
                {
                    buf << cur;
                }
                break;
                
            }
            }
        }
        outbuf_(buf, result);
    }
}
}