#include "base/config.h"
#include "base/log.h"
#include "base/thread.h"
static lane::Logger::ptr g_logger = LANE_LOG_NAME("system");
void test_lock() {
    lane::ConfigVarMgr::GetInstance()->loadFromFile("log.yml");
    lane::Thread::ptr t1(new lane::Thread(
        []() {
            while (true) {
                LANE_LOG_DEBUG(g_logger) << "---------------------------------"
                                            "------------------------";
            }
        },
        "t1"));

    lane::Thread::ptr t2(new lane::Thread(
        []() {
            while (true) {
                LANE_LOG_DEBUG(g_logger) << "+++++++++++++++++++++++++++++++++"
                                            "++++++++++++++++++++++++";
            }
        },
        "t2"));

    t1->join();
    t2->join();
}
void test_log() {
    // LANE_LOG_DEBUG(LANE_LOG_ROOT()) << "hello";
    LANE_LOG_DEBUG(LANE_LOG_NAME("system")) << "system hello";
}

void debug_log() {
    LANE_ASSERT2(false, "in func debug_log");
}
void func1() {
    debug_log();
}

void func2() {
    func1();
}
int main() {
    // lane::Logger::ptr logger(new lane::Logger());
    // lane::LogFormatter::ptr formatter(new
    // lane::LogFormatter("[%p]%T[%N:%T%t]%T[Fiber ID :%T%f]%T%F:%l%n"));
    // logger->setFormatter(formatter);
    // logger->addAppender(lane::LogAppender::ptr(new
    // lane::StdoutLogAppender())); LANE_LOG_DEBUG(logger) << "hello logger";
    // test_log();

    // test_lock();
    func2();
    return 0;
}