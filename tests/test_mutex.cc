#include <unistd.h>

#include <functional>

#include "base/iomanager.h"
#include "base/log.h"
#include "base/mutex.h"
static lane::Logger::ptr g_logger = LANE_LOG_NAME("system");
class FiberSemCallBack {
   public:
    FiberSemCallBack() : m_fs(0) {
        // LANE_LOG_INFO(g_logger) << "FiberSemCallBack()";
    }
    ~FiberSemCallBack() {
        // LANE_LOG_INFO(g_logger) << "~FiberSemCallBack()";
    }

    void waitCB() {
        LANE_LOG_INFO(g_logger) << "entry wait";
        if (m_fs.waitForSeconds(5) == true) {
            LANE_LOG_INFO(g_logger)
                << "time out, " << "sem = " << (int)m_fs.getSem();
        } else {
            LANE_LOG_INFO(g_logger)
                << "get sem," << "sem = " << (int)m_fs.getSem();
        }
        LANE_LOG_INFO(g_logger) << "exit wait";

        sleep(3);
        LANE_LOG_INFO(g_logger) << "waitCB return";
    }

    void postCB() {
        LANE_LOG_INFO(g_logger) << "entry sleep";
        sleep(3);
        LANE_LOG_INFO(g_logger) << "exit sleep";

        LANE_LOG_INFO(g_logger) << "entry post";
        m_fs.post();
        LANE_LOG_INFO(g_logger) << "exit post";

        sleep(5);

        LANE_LOG_INFO(g_logger) << "postCB return";
    }

    void producer() {
        sleep(1);
        LANE_LOG_INFO(g_logger) << "producer ready to get lock";
        lane::FiberMutex::Lock lock(m_fm);
        LANE_LOG_INFO(g_logger) << "producer get lock";
    }

    void comsumer() {
        lane::FiberMutex::Lock lock(m_fm);
        sleep(3);
        lock.unlock();
        LANE_LOG_INFO(g_logger) << "comsumer release lock";
        sleep(1);
        lock.lock();
        LANE_LOG_INFO(g_logger) << "comsumer get lock";
    }

   private:
    lane::FiberSemaphore m_fs;
    lane::FiberMutex     m_fm;
};

static FiberSemCallBack fsc;
int                     main() {
    // g_logger->setLevel(lane::LogLevel::INFO);
    lane::IOManager iom(1, "test_mutex", true);
    // iom.addTask(std::bind(
    //     (void(FiberSemCallBack::*)(void)) & FiberSemCallBack::waitCB, &fsc));

    // iom.addTask(std::bind(
    //     (void(FiberSemCallBack::*)(void)) & FiberSemCallBack::postCB, &fsc));
    iom.addTask(std::bind(
        (void(FiberSemCallBack::*)(void)) & FiberSemCallBack::comsumer, &fsc));

    iom.addTask(std::bind(
        (void(FiberSemCallBack::*)(void)) & FiberSemCallBack::producer, &fsc));
    return 0;
}