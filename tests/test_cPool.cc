#include <iostream>

#include "base/cPool.h"
#include "base/iomanager.h"
#include "base/log.h"
#include "base/mutex.h"
#include "base/waitGroup.h"

void TestMain() {
    lane::IOManager iom(2, "cpool", false);
    LANE_LOG_NAME("system")->setLevel(lane::LogLevel::FATAL);
    LANE_LOG_INFO(LANE_LOG_ROOT()) << "start";
    iom.addTask([]() {
        LANE_LOG_INFO(LANE_LOG_ROOT()) << "start pool";
        lane::CPoll pool(2);
        auto        wait = new lane::WaitGroup;
        int         num = 4;
        wait->add(num);
        for (int i = 0; i < num; ++i) {
            pool.submit([=]() {
                sleep(1);
                LANE_LOG_INFO(LANE_LOG_ROOT()) << "finish";
                wait->done();
            });
        }
        wait->wait();
        LANE_LOG_INFO(LANE_LOG_ROOT()) << "end";
    });
}


int main() {
    TestMain();
    return 0;
}