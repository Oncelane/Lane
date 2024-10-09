/*******************************************
 * Author : Lane
 * Email: : 1657015850@qq.com
 * CreateTime : 2023-01-31 19:05
 * LastModified : 2023-01-31 19:05
 * Filename : fiber.h
 * Description : 1.0 *
 ************************************/

#ifndef __LANE_FIBER_H__
#define __LANE_FIBER_H__
#include <stdint.h>
#include <stdlib.h>
#include <ucontext.h>

#include <atomic>
#include <boost/context/detail/fcontext.hpp>
#include <exception>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>

#include "base/log.h"
#include "base/macro.h"
namespace lane {
class Fiber : public std::enable_shared_from_this<Fiber> {
private:
    struct Defer {
        std::function<void()> cb;
        Defer*                next;
    };

public:
    typedef std::shared_ptr<Fiber>    ptr;
    typedef std::function<void(void)> CallBackType;
    enum State { INIT, EXEC, HOLD, READY, TERM, EXCE };

public:
    Fiber(CallBackType cb, bool withThread = false, uint32_t stackSize = 0);
    Fiber();
    ~Fiber();

public:
    void  swapIn();
    State getState() {
        return m_state;
    };
    uint32_t getId() {
        return m_id;
    };
    void                          reset(CallBackType cb);
    void                          _defer(std::function<void()> cb);
    std::optional<std::exception> _recovery();
    void                          _panic(std::exception e);

private:
    void swapOut();

public:
    static void       MainFun(boost::context::detail::transfer_t);
    static void       YieldToHold();
    static void       YieldToReady();
    static Fiber::ptr GetThis();
    static Fiber::ptr GetBefore();
    static void       SetThis(Fiber::ptr fiberPtr);
    static uint32_t   GetFiberId();

private:
    CallBackType                       m_cb;
    State                              m_state;
    uint32_t                           m_id;
    uint32_t                           m_stackSize;
    boost::context::detail::fcontext_t m_ctx;
    struct Defer*                      m_defer;
    char*                              m_stackPtr;
    bool                               m_withThread;
    std::list<std::exception>          m_exections;
    bool                               m_panic;
    bool                               m_defer_panic;
};
}  // namespace lane

#endif