#include "base/cPool.h"

#include "base/iomanager.h"
#include "base/log.h"
#include "base/mutex.h"
namespace lane {
static Logger::ptr g_logger = LANE_LOG_NAME("system");
CPoll::CPoll(uint32_t size) : size(size), m_sem(0), m_stop(false) {
    for (uint32_t i = 0; i < size; i++) {
        IOManager::GetThis()->addTask([this]() {
            while (!m_stop) {
                m_sem.wait();
                if (m_stop) {
                    return;
                }
                std::function<void()> task;
                {
                    MutexType::Lock lock(m_mu);
                    if (m_queue.empty()) {
                        continue;
                    }
                    task = m_queue.front();
                    m_queue.pop();
                }
                task();
            }
        });
    }
}

CPoll::~CPoll() {
    LANE_LOG_DEBUG(g_logger) << "~cpool()";
}

void CPoll::submit(std::function<void(void)> cb) {
    {
        MutexType::Lock lock(m_mu);
        m_queue.emplace(cb);
    }
    m_sem.post();
}

}  // namespace lane