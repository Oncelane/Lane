#include <iostream>

#include "base/iomanager.h"
#include "base/log.h"
#include "base/mutex.h"
#include "base/waitGroup.h"
int  numReaders = 200;
int  numWriters = 200;
int  iterations = 1000;
auto lock = new lane::Mutex;
// auto lock = new lane::FiberMutex;
auto wait = new lane::WaitGroup;
auto data = 0;

void TestMain() {
    // auto rwLock = new lane::FiberRWMutex;


    // 启动 numWrites 个写协程
    wait->add(numWriters);
    for (int i = 0; i < numWriters; ++i) {
        lane::IOManager::GetThis()->addTask([]() {
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
    wait->add(numReaders);
    for (int i = 0; i < numReaders; ++i) {
        lane::IOManager::GetThis()->addTask([]() {
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

    {
        lane::IOManager iom(4, "rwlock", true);
        LANE_LOG_NAME("system")->setLevel(lane::LogLevel::FATAL);
        iom.addTask(TestMain);
    }
    LANE_LOG_INFO(LANE_LOG_ROOT()) << "test finish";

    return 0;
}