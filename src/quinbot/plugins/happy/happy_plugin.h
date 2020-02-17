#pragma once

#include "../../plugin.h"
#include "../../bot.h"

namespace quinbot
{
namespace plugin
{
    class FingerGuessCommand final : public command::PublicCommand
    {
    public:
        using eExecuteResult = command::eExecuteResult;
        FingerGuessCommand()
            :   command::PublicCommand("finger_guess")
        {
            set_aliases({"石头剪刀布", "猜拳"});
            set_help_message("格式 (finger_guess/石头剪刀布/猜拳) [石头/剪刀/布]");
        }

        eExecuteResult process( const command::CommandInfo &info ) override
        {
            auto &cl = info.command_line;
            if (cl.size() == 0)
                return eExecuteResult::USER_ERROR;
            if (cl[0] != "石头" && cl[0] != "剪刀" && cl[0] != "布")
            {
                info.send_back("？出的什么看不懂");
                return eExecuteResult::USER_ERROR;
            }
            if (cl[0] ==  "石头")
                info.send_back("布");
            else if (cl[0] == "剪刀")
                info.send_back("石头");
            else if (cl[0] == "布")
                info.send_back("剪刀");
            info.send_back("我赢了，我必不可能输");
            return eExecuteResult::SUCCESS;
        }
    };

    class BanCommand final : public command::PublicCommand
    {
        using eExecuteResult = command::eExecuteResult;
    public:
        BanCommand()
            :   command::PublicCommand("ban")
        {
            set_aliases({"口他"});
        }

        command::eExecuteResult process( const command::CommandInfo &info ) override
        {
            auto &logger = bot.get_logger();
            auto &cl = info.command_line;

            if (cl.size() == 0)
            {
                info.send_back("好小");
                return command::eExecuteResult::USER_ERROR;
            }

            int64_t user_id = 0;
            cq::message::Message msg(cl[0]);
            
            for (const auto &seg : msg)
            {
                if (seg.type == "at")
                {
                    user_id = std::stoll(seg.data.at("qq"));
                    if (user_id == 1748065414 || user_id == 2130213628)
                    {
                        info.send_back("[CQ:image,file=A3131143FB5C0DCE3EE1394A4775E6DF.jpg]");
                        return command::eExecuteResult::SUCCESS;
                    }
                    cq::api::set_group_ban(info.get_group_id(), user_id, 60);
                    info.send_back("咕噜咕噜咕噜咕噜[CQ:image,file=69AEB5C57A9F71A0AA042C34E9FAB6A2.jpg]");
                    return command::eExecuteResult::SUCCESS;
                }
                else
                {
                    info.send_back("好小");
                    return command::eExecuteResult::USER_ERROR;
                }
            }

            info.send_back(msg);
            return command::eExecuteResult::SUCCESS;
        }
    };

    /*class RussianLoading : public command::PublicCommand
    {
        using eExecuteResult = command::eExecuteResult;
    public:
        RussianLoading()
            :   command::PublicCommand("loading")
        {
            set_aliases({"俄罗斯转盘", "转盘"});
        }

        eExecuteResult process( const command::CommandInfo &info ) override
        {
            auto &cl = info.command_line;
            if (cl.size() == 0)
            {
                info.send_back("有效参数过少 帮助参考 #help loading");
                return eExecuteResult::USER_ERROR;
            }
            std::string sub_func = cl[0];
            if (util::in(sub_func, {"start", "开始", "start!", "开始!"}))
                return process_start(info);
            else if (util::in(sub_func, {"join", "加入"}))
                return process_join(info);
            else if (util::in(sub_func, {"fire", "崩"}))
                return process_fire(info);
        }
    
    private:
        enum State { START, WAIT };
        struct GameInfo { State state; int32_t max_size, current_turn, over_turn; std::vector<int64_t> players; };
        
        std::unordered_map<int64_t, GameInfo> games;

        eExecuteResult process_start( const command::CommandInfo &info )
        {
            command::ArgsMap args = command::arg_parse(info.command_line,
            {
                {"size", {"人数", "参与玩家"}}
            },
            {});

            int32_t size = 0;
            try { size = args.get<int32_t>("size", 6, true); }
            catch ( const std::underflow_error & )
            {
                info.send_back("有效参数过少");
                return eExecuteResult::USER_ERROR;
            }
            catch ( const std::invalid_argument & )
            {
                info.send_back("错误的参数解析");
                return eExecuteResult::USER_ERROR;
            }

            if (size <= 1)
            {
                info.send_back("参与人数至少为2");
                return eExecuteResult::USER_ERROR;
            }

            if (size > 6)
            {
                info.send_back("参与人数至多为6");
                return eExecuteResult::USER_ERROR;
            }
            int64_t group_id = info.get_group_id();
            if (games.find(group_id) != games.end() && games[group_id].state == START)
            {
                info.send_back("游戏已经开始了！");
                return eExecuteResult::USER_ERROR;
            }

            state_ = START;
            max_size = size;
            players_.push_back(info.get_user_id());
            info.send_back("ゲームを始めましょう！[CQ:image,file=E1DA3DF7D359D128C37DA2A6273081E5.jpg]");
            info.send_back(print_players_(players_));
            info.send_back("参与玩家使用 #loading [join/加入] 加入游戏");

            return eExecuteResult::SUCCESS;
        }

        eExecuteResult process_join( const command::CommandInfo &info )
        {
            if (state_ == WAIT)
            {
                info.send_back("游戏尚未开始！");
                return eExecuteResult::USER_ERROR;
            }
            if (info. command_line.size() != 1)
            {
                info.send_back("参数过多");
                return eExecuteResult::USER_ERROR;
            }
            int64_t user_id = info.get_user_id();
            for (auto &p : players_)
            {
                if (p == user_id)
                {
                    info.send_back("你已经参加游戏了！");
                    return eExecuteResult::USER_ERROR;
                }
            }
            if (players_.size() == max_size)
            {
                info.send_back("人数已满！");
                return eExecuteResult::USER_ERROR;
            }
            players_.push_back(user_id);
            info.send_back(print_players_(players_));
            if (players_.size() == max_size)
            {
                start_game(info);
                return eExecuteResult::SUCCESS;
            }
            return eExecuteResult::SUCCESS;
        }

        eExecuteResult process_fire( const command::CommandInfo &info )
        {
            if (state_ == WAIT)
            {
                info.send_back("游戏尚未开始！");
                return eExecuteResult::USER_ERROR;
            }
            if (!util::in(info.get_user_id(), players_))
            {
                info.send_back("你没有参加游戏");
                return eExecuteResult::USER_ERROR;
            }
            ++current_turn;
            if (current_turn == over_turn)
            {
                cq::message::Message msg;
                msg += cq::message::MessageSegment::at(info.get_user_id);
                msg += cq::message::MessageSegment::text("\n你没了！游戏结束，" + cq::api::get_group_member_info(info.get_group_id(), info.get_user_id()).nickname + " R.I.P");
                info.send_back(msg);
                cq::api::set_group_ban(info.get_group_id(), info.get_user_id(), 60);
                end_game();
                return eExecuteResult::SUCCESS;
            }
        }

        void start_game( const command::CommandInfo &info )
        {
            info.send_back("游戏开始！使用 #loading [fire/邦]开火！");
            over_turn = util::range_random(1, max_size);
        }

        void end_game()
        {
            current_turn = 0;
            over_turn = 0;
            max_size = 0;
            players_.clear();
            state_ = WAIT;
        }

        std::string print_players_( const std::vector<int64_t> &players ) const
        {
            cq::message::Message msg;
            msg += cq::message::MessageSegment::text("参与人数(" + std::to_string(players.size()) + "/4)\n");
            for (int i = 0; i < players.size(); ++i)
            {
                msg += cq::message::MessageSegment::text("[" + std::to_string(i + 1) + "]");
                msg += cq::message::MessageSegment::at(players[i]);
                msg += cq::message::MessageSegment::text("\n");
            }
            return msg;
        }

    };*/

    class HappyPlugin final : public Plugin
    {
    public:
        HappyPlugin()
            :   Plugin("happy")
        {
        }

        void on_enable() override
        {
            quinbot::bot.get_command_manager()->register_command<FingerGuessCommand>();
            quinbot::bot.get_command_manager()->register_command<BanCommand>();
        }
        
    private:

    };
}
}