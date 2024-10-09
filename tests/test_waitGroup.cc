#include <unistd.h>

#include <iostream>

#include "base/iomanager.h"
#include "base/laneGo.h"
#include "base/log.h"
#include "base/mutex.h"
#include "base/waitGroup.h"

void TestWaitGroup() {
    info() << "start runing";
    auto wait = new lane::WaitGroup;

    for (int i = 0; i < 5; ++i) {
        wait->add(1);
        co[=]() {
            defer[&]() {
                wait->done();
            };
            sleep(1);
            info() << " fast finish";
        };
    }
    wait->add(1);
    co[=]() {
        defer[&]() {
            wait->done();
        };
        sleep(2);
        info() << " slow finish";
    };

    wait->wait();
    info() << "end";
}

void TestMain() {
    lane::IOManager iom(2, "waitgroup", false);
    iom.addTask(TestWaitGroup);
}


int main() {
    TestMain();
    return 0;
}