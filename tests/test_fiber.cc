#include "base/fiber.h"

lane::Logger::ptr g_logger = LANE_LOG_NAME("system");
void test() {
    lane::Fiber::ptr fp(new lane::Fiber(
        []() {
            LANE_LOG_DEBUG(g_logger) << "fiber fun begin";
            lane::Fiber::YieldToReady();
            LANE_LOG_DEBUG(g_logger) << "fiber fun end";
        },
        true));
    lane::Fiber::GetThis();

    fp->swapIn();
    LANE_LOG_DEBUG(g_logger) << "fiber fun out";
    fp->swapIn();
}

int main() {
    test();

    LANE_LOG_DEBUG(g_logger) << "main over";
    return 0;
}