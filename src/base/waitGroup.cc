#include "base/waitGroup.h"
namespace lane {
void WaitGroup::add(uint count) {
    MutexType::Lock lock(m_mutex);
    m_sem += count;

    LANE_ASSERT(m_sem >= 0)

    // 若==0 唤醒阻塞队列的全部成员
    if (m_sem == 0) {
        while (!m_waitQueue.empty()) {
            auto it = m_waitQueue.front();
            m_waitQueue.pop();
            it.first->schedule(it.second);
        }
    }
}

void WaitGroup::wait() {
    MutexType::Lock lock(m_mutex);
    m_waitQueue.emplace(IOManager::GetThis(), Fiber::GetThis());
    lock.unlock();
    Fiber::YieldToHold();
}

void WaitGroup::done() {
    add(-1);
}

}  // namespace lane