#include "base/waitGroup.h"

#include "base/log.h"
namespace lane {
WaitGroup::~WaitGroup() {
    add(0);
    wait();
    add(0);
}
void WaitGroup::add(int count) {
    MutexType::Lock lock(m_mutex);
    m_sem += count;
    LANE_LOG_DEBUG(LANE_LOG_ROOT())
        << "wg add:" << count << " now m_sem=" << m_sem;
    LANE_ASSERT(m_sem >= 0)

    // 若==0 唤醒阻塞队列的全部成员
    if (m_sem == 0) {
        while (!m_waitQueue.empty()) {
            auto it = m_waitQueue.front();
            m_waitQueue.pop();
            it.first->addTask(it.second);
            LANE_LOG_DEBUG(LANE_LOG_ROOT()) << "wg pop:" << it.second;
            it.first->delBlock();
        }
    }
}

void WaitGroup::wait() {
    add(0);
    MutexType::Lock lock(m_mutex);
    if (m_sem == 0) {
        return;
    }
    IOManager::GetThis()->addBlock();
    LANE_LOG_DEBUG(LANE_LOG_ROOT()) << "wg emplace:" << Fiber::GetThis();
    m_waitQueue.emplace(IOManager::GetThis(), Fiber::GetThis());
    lock.unlock();
    Fiber::YieldToHold();
}

void WaitGroup::done() {
    add(-1);
}

}  // namespace lane