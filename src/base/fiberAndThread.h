
#pragma once
#include <functional>
#include <memory>

#include "base/fiber.h"
#include "base/mutex.h"
// #include "base/thread.h"
namespace lane {
class Fiber;
struct FiberAndThread {
    // typedef std::shared_ptr<Fiber> ptr;
    using MutexType = SpinMutex;

public:
    std::shared_ptr<Fiber>    m_fiber;
    std::function<void(void)> m_cb;
    pid_t                     m_tId;

public:
    FiberAndThread() : m_fiber(nullptr), m_cb(nullptr), m_tId(-1){};
    FiberAndThread(std::shared_ptr<Fiber> f, pid_t tId)
        : m_fiber(f), m_cb(nullptr), m_tId(tId) {}

    FiberAndThread(std::shared_ptr<Fiber>* fp, pid_t tId)
        : m_cb(nullptr), m_tId(tId) {
        swap(m_fiber, *fp);
    }

    FiberAndThread(std::function<void(void)> cb, pid_t tId)
        : m_fiber(nullptr), m_cb(cb), m_tId(tId) {}

    FiberAndThread(std::function<void(void)>* cb, pid_t tId)
        : m_fiber(nullptr), m_tId(tId) {
        swap(m_cb, *cb);
    }
};
}  // namespace lane
