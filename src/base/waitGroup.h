#ifndef __LANE_WAITGROUP_H__
#define __LANE_WAITGROUP_H__

#include <sys/types.h>

#include <queue>

#include "base/fiber.h"
#include "base/iomanager.h"
#include "base/macro.h"
#include "base/mutex.h"

namespace lane {
class WaitGroup {
public:
    WaitGroup() {}
    ~WaitGroup() {}
    void add(uint count) {
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

    // 入阻塞队列
    void wait() {
        MutexType::Lock lock(m_mutex);
        m_waitQueue.emplace(IOManager::GetThis(), Fiber::GetThis());
        lock.unlock();
        Fiber::YieldToHold();
    }
    void done() {
        add(-1);
    }

private:
    using MutexType = Mutex;
    std::queue<std::pair<IOManager*, std::shared_ptr<Fiber>>> m_waitQueue;
    uint                                                      m_sem;
    MutexType                                                 m_mutex;  // 因为需要对std::list进行增删查改，所以必须用互斥锁
};
}  // namespace lane


#endif