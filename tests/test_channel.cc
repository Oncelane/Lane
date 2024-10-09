
#include <unistd.h>

#include <atomic>
#include <iostream>

#include "base/channel.h"
#include "base/laneGo.h"
#include "base/log.h"
#include "base/mutex.h"
#include "base/waitGroup.h"

static std::atomic_int g_int = 0;

void producer(lane::Channel<int>* ch) {
    while (true) {
        if (g_int > 4) {
            ch->close();
            break;
        }
        if (!ch->input(g_int++)) {
            break;
        }
    }
    info() << "producer exit";
}

void comsumer(lane::Channel<int>* ch) {
    ch->range([=](int elem) {
        // do something
        info() << elem;
    });
    info() << "consumer exit";
}

void TestConsumer(lane::Channel<int>* ch) {
    for (int i = 0; i < 3; ++i) {
        co std::bind(producer, ch);
    }
    co std::bind(comsumer, ch);
}

void TestMain() {
    auto*           ch = new lane::Channel<int>(0);
    lane::IOManager iom(4, "test", false);
    iom.addTask(std::bind(TestConsumer, ch));
}

void TestUnbufferChannelMain() {
    while (true) {
        g_int = 0;
        auto*           ch = new lane::Channel<int>(0);
        lane::IOManager iom(4, "test", false);
        iom.addTask(std::bind(TestConsumer, ch));
        std::cout << "==================end test===============" << std::endl;
        delete ch;
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

void TestPrint() {
    auto wg = new lane::WaitGroup;

    // 创建两个 channel，分别用于控制字母和数字的打印
    auto letters = lane::Channel<bool>(0);
    auto numbers = lane::Channel<bool>(0);

    // 字母打印协程
    wg->add(1);
    co[&]() {
        defer[&]() {
            wg->done();
        };
        for (char ch = 'A'; ch <= 'C'; ch++) {
            info() << ch;  // 打印字母
            letters.input(true);  // 发送信号，通知数字打印协程可以打印
            numbers.output();  // 等待数字打印协程的信号
        }
        letters.close();  // 关闭字母 channel
    };


    // 数字打印协程
    wg->add(1);
    co[&]() {
        defer[&]() {
            wg->done();
        };
        for (int num = 1; num <= 3; num++) {
            letters.output();  // 等待字母打印协程的信号
            info() << num;     // 打印数字
            numbers.input(true);
        }
        numbers.close();  // 关闭数字 channel
    };

    wg->wait();  // 等待所有协程完成
}


int main() {
    lane::IOManager iom(2, "channel", false);
    iom.addTask(TestPrint);
    return 0;
}