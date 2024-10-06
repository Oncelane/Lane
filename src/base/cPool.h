#ifndef __LANE_CPOOL_H__
#define __LANE_CPOOL_H__
#include <sys/types.h>

#include <functional>
#include <ostream>
#include <queue>

#include "base/mutex.h"
namespace lane {
class CPoll {
public:
    CPoll(uint32_t size);
    ~CPoll();
    void start() {}
    void stop() {
        m_stop = true;
    }
    void          submit(std::function<void(void)> cb);
    std::ostream& dump(std::ostream& os) {
        os << " size:" << size;
        return os;
    }

private:
    using MutexType = FiberMutex;
    uint32_t                              size;
    std::queue<std::function<void(void)>> m_queue;
    FiberSemaphore                        m_sem;
    MutexType                             m_mu;
    bool                                  m_stop;
};
}  // namespace lane

#endif