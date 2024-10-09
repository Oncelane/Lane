#include <unistd.h>

#include <functional>

#include "base/iomanager.h"
#include "base/log.h"
#include "base/mutex.h"
static lane::Logger::ptr g_logger = LANE_LOG_NAME("system");
static int               data = 0;
static int               num = 1000;
class FiberSemCallBack {
public:
    FiberSemCallBack() {
        // LANE_LOG_INFO(g_logger) << "FiberSemCallBack()";
    }
    ~FiberSemCallBack() {
        // LANE_LOG_INFO(g_logger) << "~FiberSemCallBack()";
    }

    void producer() {

        // LANE_LOG_INFO(g_logger) << "producer ready to get lock";
        for (int i = 0; i < num; i++) {
            MutexType::Lock lock(m_mu);
            data++;
        }
        // LANE_LOG_INFO(g_logger) << "producer get lock";
    }

    void comsumer() {
        // LANE_LOG_INFO(g_logger) << "comsumer release lock";
        for (int i = 0; i < num; i++) {
            MutexType::Lock lock(m_mu);
            data++;
        }

        // LANE_LOG_INFO(g_logger) << "comsumer get lock";
    }

private:
    using MutexType = lane::FiberMutex;
    MutexType m_mu;
};


static FiberSemCallBack fsc;

int main() {
    {
        g_logger->setLevel(lane::LogLevel::FATAL);
        lane::IOManager iom(4, "test_mutex", false);
        // iom.addTask(std::bind(
        //     (void(FiberSemCallBack::*)(void)) & FiberSemCallBack::waitCB,
        //     &fsc));

        // iom.addTask(std::bind(
        //     (void(FiberSemCallBack::*)(void)) & FiberSemCallBack::postCB,
        //     &fsc));
        iom.addTask(std::bind(
            (void (FiberSemCallBack::*)(void)) & FiberSemCallBack::comsumer,
            &fsc));

        iom.addTask(std::bind(
            (void (FiberSemCallBack::*)(void)) & FiberSemCallBack::comsumer,
            &fsc));
        iom.addTask(std::bind(
            (void (FiberSemCallBack::*)(void)) & FiberSemCallBack::producer,
            &fsc));
    }
    LANE_LOG_FATAL(g_logger) << data;
    return 0;
}