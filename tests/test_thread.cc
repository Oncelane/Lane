#include <unistd.h>

#include <iostream>

#include "base/log.h"
#include "base/thread.h"
static lane::Logger::ptr g_logger = LANE_LOG_NAME("system");
int main() {
    lane::Thread::ptr t1(new lane::Thread(
        []() {
            ::sleep(1);
            LANE_LOG_INFO(g_logger) << "t1";
        },
        ""));

    // lane::Thread::ptr t2 = t1;
    // LANE_LOG_DEBUG(g_logger) << "t1:"<< t1.use_count() << "--" <<
    // t2.use_count() << "--" << t1->shared_from_this().use_count();
    // lane::Thread::ptr t2(new lane::Thread([](){
    //     LANE_LOG_INFO(g_logger) << "t2";
    // }, "t2"));
    // char  c;
    // std::cin>> c;
    t1->join();
    ::sleep(1);
    LANE_LOG_INFO(g_logger) << "over";

    pthread_exit(0);
    // return 0;
}
