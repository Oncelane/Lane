#include <time.h>

#include <memory>

#include "base/hook.h"
#include "base/scheduler.h"
lane::Logger::ptr g_logger = LANE_LOG_NAME("system");
void              fun() {
                 lane::set_hook_enable(false);
                 static int count = 0;
                 LANE_LOG_DEBUG(g_logger) << "fun:" << count;
                 if (count < 5) {
                     lane::Scheduler::GetThis()->schedule(fun);
                     count++;
    }
                 sleep(1);
}
void fun2() {
    lane::set_hook_enable(false);
    static int count = 0;
    LANE_LOG_DEBUG(g_logger) << "fun:" << count;
    count++;
    sleep(2);
}

void test1() {
    lane::Scheduler::ptr sch(new lane::Scheduler(2, "test_sche"));

    sch->start();
    sch->schedule(fun);
    sch->stop();
    LANE_LOG_DEBUG(g_logger) << "end";
}

void test2() {
    std::unique_ptr<lane::Scheduler> sch(
        new lane::Scheduler(3, "schedule", false));
    sch->schedule([]() {
        LANE_LOG_DEBUG(g_logger) << "fun begin";

        while (true) {
            static int count = 0;
            if (count < 100) {
                lane::Fiber::YieldToReady();
                LANE_LOG_DEBUG(g_logger) << "fun ready back :" << count++;
            } else {
                break;
            }
        }
        LANE_LOG_DEBUG(g_logger) << "fun end";
    });

    sch->start();

    sch->stop();
    LANE_LOG_DEBUG(g_logger) << "test end";
}

void test3() {
    lane::Scheduler sch(2, "schdule");
    sch.start();
    for (int i = 0; i < 10; ++i) {
        sch.schedule(fun2);
    }
    sch.stop();
}

void fun4() {
    lane::set_hook_enable(false);
    static int  count = 0;
    static bool flag = false;
    if (flag == 0) {
        flag = true;
        for (int i = 0; i < 100; ++i) {
            lane::Scheduler::GetThis()->schedule(fun4, lane::Thread::GetTid());
        }
    }
    LANE_LOG_DEBUG(g_logger) << "fun:" << count++;
    sleep(1);
}

// 测试steal
void test4() {
    lane::Scheduler sch(4, "schdule");
    sch.start();
    // for(int i = 0; i < 10; ++i){
    sch.schedule(fun4);
    // }
    sch.stop();
}

// 每个协程一百万次调度
static int count = 0;
void       test_fiber_yield() {
          for (int i = 0; i < 1000000; ++i) {
              count++;
              lane::Fiber::YieldToReady();
              count--;
    }
}

// 五个协程任务
void test_yield_speed() {
    lane::Scheduler::GetThis()->addTask(std::bind(&test_fiber_yield));
    lane::Scheduler::GetThis()->addTask(std::bind(&test_fiber_yield));
    lane::Scheduler::GetThis()->addTask(std::bind(&test_fiber_yield));
    lane::Scheduler::GetThis()->addTask(std::bind(&test_fiber_yield));
    lane::Scheduler::GetThis()->addTask(std::bind(&test_fiber_yield));
}

// 两个线程提供服务
void test5() {
    LANE_LOG_INFO(g_logger) << "test begin";
    lane::Scheduler sch(2, "test_speed");
    sch.start();
    sch.addTask(test_yield_speed);
    sch.stop();
    LANE_LOG_INFO(g_logger) << "test end";
}

void many_test5() {
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 10; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        test5();
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        LANE_LOG_INFO(g_logger) << "Test " << i << " spend" << elapsed.count();
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    LANE_LOG_INFO(g_logger) << "Test Average"
                            << " spend" << elapsed.count() / 10 << "second";
}

void test6_content() {
    for (int i = 0; i < 500; ++i) {
        // 一些计算操作，例如乘法
        int result = i * i;
        lane::Fiber::YieldToReady();
    }
}
void test6_im() {
    for (int i = 0; i < 10000; ++i) {
        lane::Scheduler::GetThis()->addTask(std::bind(&test6_content));
    }
}

void test6() {
    LANE_LOG_INFO(g_logger) << "test begin";
    lane::Scheduler sch(2, "test_speed");
    sch.start();
    sch.addTask(test6_im);
    sch.stop();
    LANE_LOG_INFO(g_logger) << "test end";
}

void many_test6() {
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 10; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        test6();
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        LANE_LOG_INFO(g_logger) << "Test " << i << " spend" << elapsed.count();
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    LANE_LOG_INFO(g_logger) << "Test Average"
                            << " spend" << elapsed.count() / 10 << "second";
}
int main() {
    g_logger->setLevel(lane::LogLevel::INFO);
    // test1();
    // test2();
    many_test5();
}