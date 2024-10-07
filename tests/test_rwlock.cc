#include <iostream>

#include "base/iomanager.h"
#include "base/log.h"
#include "base/mutex.h"
#include "base/waitGroup.h"
int numReaders = 100;
int numWriters = 100;
int iterations = 10000;


void TestMain() {
    // auto rwLock = new lane::FiberRWMutex;
    auto lock = new lane::FiberMutex;
    // auto lock = new lane::Mutex;
    auto wait = new lane::WaitGroup;
    auto data = 0;

    // 启动 numWrites 个写协程
    for (int i = 0; i < numWriters; ++i) {
        wait->add(1);
        lane::IOManager::GetThis()->addTask([=, &data]() {
            for (int i = 0; i < iterations; ++i) {
                // rwLock->lock();
                lock->lock();
                data++;
                lock->unlock();
                // rwLock->unlock();
            }
            wait->done();
            // LANE_LOG_INFO(LANE_LOG_ROOT()) << "write end";
        });
    }

    // 启动 numReaders 个读协程
    for (int i = 0; i < numReaders; ++i) {
        wait->add(1);
        lane::IOManager::GetThis()->addTask([=]() {
            for (int i = 0; i < iterations; ++i) {
                // rwLock->rLock();
                // LANE_LOG_INFO(LANE_LOG_ROOT()) << data;
                // rwLock->rUnlock();
                lock->lock();
                lock->unlock();
            }
            wait->done();
            // LANE_LOG_INFO(LANE_LOG_ROOT()) << "read end";
        });
    }
    LANE_LOG_INFO(LANE_LOG_ROOT()) << "gen task finish";
    auto start = std::chrono::high_resolution_clock::now();
    wait->wait();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    LANE_LOG_INFO(LANE_LOG_ROOT())
        << "data = " << data << " Test spend " << elapsed.count() << " s";
}

int main() {

    lane::IOManager iom(4, "rwlock", false);
    LANE_LOG_NAME("system")->setLevel(lane::LogLevel::FATAL);
    iom.addTask(TestMain);
    // std::cin.get();

    return 0;
}