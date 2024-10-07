
#include <unistd.h>

#include <iostream>

#include "base/channel.h"
#include "base/iomanager.h"
#include "base/log.h"

static int g_int = 0;

void producer(lane::Channel<int>* ch, int index) {
    while (true) {
        if (g_int > 11) {
            ch->close();
            break;
        }
        LANE_LOG_INFO(LANE_LOG_ROOT()) << index << " in g_int=" << g_int;
        if (!ch->input(g_int++)) {
            break;
        }
    }
    LANE_LOG_INFO(LANE_LOG_ROOT()) << "producer exit";
}

void comsumer(lane::Channel<int>* ch, int index) {
    // static int count = 0;
    ch->range([=](int elem) {
        LANE_LOG_INFO(LANE_LOG_ROOT()) << index << " out g_int=" << elem;
        // if (count++ > 11) {
        //     ch->close();
        // }
    });
    LANE_LOG_INFO(LANE_LOG_ROOT()) << "consumer exit";
}

void TestConsumer(lane::Channel<int>* ch) {
    lane::IOManager iom(4, "test", false);
    LANE_LOG_NAME("system")->setLevel(lane::LogLevel::FATAL);

    for (int i = 0; i < 3; ++i) {
        iom.addTask(std::bind(comsumer, ch, i));
        iom.addTask(std::bind(producer, ch, i));
    }
}

void TestUnbufferChannelMain() {
    while (true) {
        g_int = 0;
        auto* ch = new lane::Channel<int>(0);
        TestConsumer(ch);
        std::cout << "==================end test===============" << std::endl;
        delete ch;
        // std::cin.get();
    }
}

void TestConflatedChannelMain() {
    while (true) {
        g_int = 0;
        auto* ch = new lane::Channel<int>(lane::Channel<int>::CONFLATED);
        TestConsumer(ch);
        std::cout << "==================end test===============" << std::endl;
        delete ch;
    }
}


int main() {

    TestUnbufferChannelMain();
    return 0;
}