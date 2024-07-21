#include "base/iomanager.h"
#include "base/log.h"
#include "init/daemon.h"

static lane::Logger::ptr g_logger = LANE_LOG_ROOT();

lane::Timer::ptr timer;
int server_main(int argc, char** argv) {
    LANE_LOG_INFO(g_logger) << lane::ProcessInfoMgr::GetInstance()->toString();
    lane::IOManager iom(1);
    timer = iom.addTimer(
        1000,
        [&iom]() {
            LANE_LOG_INFO(g_logger) << "onTimer";
            static int count = 0;
            if (++count > 1) {
                // exit(1);
                timer->cancel();
                return;
            }
        },
        true);
    return 0;
}

int main(int argc, char** argv) {
    return lane::start_daemon(argc, argv, server_main, argc != 1);
}
