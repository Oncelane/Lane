#include <unistd.h>

#include <iostream>

#include "base/log.h"
#include "base/mutex.h"
#include "base/waitGroup.h"

lane::WaitGroup wait;

void TestWaitGroup() {
    LANE_LOG_INFO(LANE_LOG_ROOT()) << "start runing";
    wait.add(6);
    for (int i = 0; i < 5; ++i) {
        lane::IOManager::GetThis()->addTask([=]() {
            sleep(1);
            LANE_LOG_INFO(LANE_LOG_ROOT()) << i << " finish";
            wait.done();
        });
    }
    lane::IOManager::GetThis()->addTask([=]() {
        sleep(2);
        LANE_LOG_INFO(LANE_LOG_ROOT()) << 100 << " finish";
        wait.done();
    });
    wait.wait();
    LANE_LOG_INFO(LANE_LOG_ROOT()) << "end";
}

void TestMain() {
    lane::IOManager iom(2, "waitgroup", false);
    LANE_LOG_NAME("system")->setLevel(lane::LogLevel::INFO);
    iom.addTask(TestWaitGroup);
    // std::cin.get();
}


int main() {
    TestMain();
    return 0;
}