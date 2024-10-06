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
    void add(uint count);
    void wait();
    void done();

private:
    using MutexType = FiberMutex;
    std::queue<std::pair<IOManager*, std::shared_ptr<Fiber>>> m_waitQueue;
    uint                                                      m_sem;
    MutexType                                                 m_mutex;  // 因为需要对std::list进行增删查改，所以必须用互斥锁
};
}  // namespace lane


#endif