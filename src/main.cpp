#include "cqsdk/cqsdk.h"

#include "quinbot/quinbot.h"

#include "quinbot/plugins/test.h"
#include "quinbot/plugins/ascii2d_search/ascii2d_search_plugin.h"
#include "quinbot/plugins/random_illustration/random_illustration_plugin.h"
#include "quinbot/plugins/repeater/repeater_plugin.h"
#include "quinbot/plugins/dialog/dialog_plugin.h"
#include "quinbot/plugins/welcome/welcome_plugin.h"
#include "quinbot/plugins/happy/happy_plugin.h"
#include "quinbot/plugins/pixiv/pixiv_plugin.h"
#include "quinbot/plugins/timer/timer.h"

#include <cstdlib>
#include <ctime>

CQ_MAIN
{
    quinbot::initialize();
    quinbot::load<quinbot::test::TestPlugin>(true);
    quinbot::load<quinbot::plugin::Ascii2dSearchPlugin>(true);
    quinbot::load<quinbot::plugin::RandomIllustrationPlugin>(true);
    quinbot::load<quinbot::plugin::RepeaterPlugin>(true);
    quinbot::load<quinbot::plugin::DialogPlugin>(true);
    quinbot::load<quinbot::plugin::WelcomePlugin>(true);
    quinbot::load<quinbot::plugin::HappyPlugin>(true);
    //quinbot::load<quinbot::plugin::PixivPlugin>(true);
    quinbot::load<quinbot::plugin::TimerPlugin>(true);
}

