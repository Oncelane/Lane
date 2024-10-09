#include "base/waitGroup.h"

#include "base/log.h"
#include "base/macro.h"
#include "base/mutex.h"
namespace lane {
WaitGroup::~WaitGroup() {
    // wait();
    // add(0);
    LANE_LOG_DEBUG(LANE_LOG_ROOT()) << "waitgroup dead";
}
void WaitGroup::add(int count) {
    MutexType::Lock lock(m_mutex);
    m_sem += count;
    // LANE_LOG_DEBUG(LANE_LOG_ROOT())
    //     << "wg add:" << count << " now m_sem=" << m_sem;
    LANE_ASSERT(m_sem >= 0)

    // 若==0 唤醒阻塞队列的全部成员
    if (m_sem == 0) {
        while (!m_waitQueue.empty()) {
            // LANE_LOG_DEBUG(LANE_LOG_ROOT())
            //     << "wg pop:" << Fiber::GetThis()
            //     << " front pc:" << &m_waitQueue.front()
            //     << " m_waitGroup Pc:" << &m_waitQueue;
            std::pair<IOManager*, Fiber::ptr> it = m_waitQueue.front();
            m_waitQueue.pop();
            it.first->addTask(it.second);
            it.first->delBlock();
        }
    }
}

void WaitGroup::wait() {
    // add(0);
    {
        MutexType::Lock lock(m_mutex);
        if (m_sem == 0) {
            return;
        }
        IOManager::GetThis()->addBlock();
        m_waitQueue.emplace(IOManager::GetThis(), Fiber::GetThis());
        // LANE_LOG_DEBUG(LANE_LOG_ROOT()) << "wg emplace:" << Fiber::GetThis()
        //                                 << " front pc:" <<
        //                                 &m_waitQueue.front()
        //                                 << " m_waitGroup Pc:" <<
        //                                 &m_waitQueue;
    }

    Fiber::YieldToHold();
}

void WaitGroup::done() {
    add(-1);
}

}  // namespace lane