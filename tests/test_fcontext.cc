#include <algorithm>
#include <boost/context/detail/fcontext.hpp>
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>

#include "base/fiber.h"
#include "base/log.h"
#include "base/scheduler.h"

using namespace boost::context::detail;

void testFiber() {
    auto f1 = lane::Fiber::ptr(new lane::Fiber(
        []() {
            printf("enter f1\n");
            printf("end f1\n");
        },
        true));

    auto f2 = lane::Fiber::ptr(new lane::Fiber(
        []() {
            printf("enter f2\n");
            printf("end f2\n");
        },
        true));

    printf("main -> f1\n");
    f1->swapIn();
    printf("f1 -> main\n");

    printf("main -> f2\n");
    f2->swapIn();
    printf("f2 -> main\n");

    printf("main exit\n");
}

uint count = 0;
uint b = 1000000;

void benchmark_fcontext() {
    lane::Fiber::GetThis();
    for (uint i = 0; i < b; ++i) {
        auto f = lane::Fiber::ptr(new lane::Fiber([]() { count++; }));
        f->swapIn();
    }
}

void benchmark_simply() {
    using namespace std::chrono;
    b = 1000000;
    std::cout << "swap count: " << b << std::endl;
    // 测试 benchmark_fcontext
    auto start = high_resolution_clock::now();
    benchmark_fcontext();
    auto end = high_resolution_clock::now();
    auto duration_fcontext = duration_cast<milliseconds>(end - start).count();
    std::cout << "benchmark_fcontext: " << duration_fcontext << " ms"
              << std::endl;
}
int  result;
void content() {
    for (int i = 0; i < 500; ++i) {
        // 一些计算操作，例如乘法
        result = i * i;
        lane::Fiber::YieldToReady();
    }
}
void fiberGenerator() {
    for (int i = 0; i < 10000; ++i) {
        lane::Scheduler::GetThis()->addTask(std::bind(&content));
    }
}

void bunchOfTask() {
    std::cout << "test begin" << std::endl;
    lane::Scheduler sch(2, "test_speed");
    sch.addTask(fiberGenerator);
    sch.start();
    sch.stop();
    std::cout << "test end" << std::endl;
}

void benchtasks(int count) {
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < count; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        bunchOfTask();
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        std::cout << "Test " << i << " spend" << elapsed.count() << std::endl;
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Test Average"
              << " spend" << elapsed.count() / count << "second" << std::endl;
}


void benchmakr_scheduler() {
    benchtasks(1);

    std::cout << "make sure task finish" << result << std::endl;
}

void debug_Sche() {
    std::cout << "test begin" << std::endl;
    lane::Scheduler sch(1, "test_speed");
    sch.addTask(content);
    sch.start();
    sch.stop();
    std::cout << "test end" << std::endl;
}
int main() {
    debug_Sche();
}