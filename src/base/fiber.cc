#include "base/fiber.h"

#include <boost/context/detail/fcontext.hpp>
#include <boost/context/detail/index_sequence.hpp>
#include <cstddef>
#include <exception>
#include <optional>
#include <stdexcept>
#include <string>

#include "base/config.h"
#include "base/laneGo.h"
#include "base/log.h"
#include "base/macro.h"
#include "base/scheduler.h"
namespace lane {
static Logger::ptr           g_logger = LANE_LOG_NAME("system");
static std::atomic<uint32_t> s_fiber_id = {0};
static std::atomic<uint32_t> s_fiber_count = {0};
// 切缘到线程的协程
static thread_local Fiber::ptr t_threadFiber(nullptr);
// 当前执行的协程
static thread_local Fiber::ptr t_fiber(nullptr);
// before协程 用于 defer恢复
static thread_local Fiber::ptr t_before(nullptr);

static ConfigVar<uint32_t>::ptr g_stack_size =
    ConfigVarMgr::GetInstance()->lookUp("fiber.stackSize",
                                        uint32_t(1024 * 128),
                                        "fiber's stack size");

class Allocator {
public:
    static void* Alloc(size_t size) {
        return malloc(size);
    }

    static void Dealloc(void* vp, size_t size) {
        return free(vp);
    }
};


// *****************Fiber**************

Fiber::Fiber(CallBackType cb, bool withThread, uint32_t stackSize)
    : m_cb(cb)
    , m_defer(nullptr)
    , m_withThread(withThread)
    , m_panic(false)
    , m_defer_panic(false) {
    // lane::Fiber::GetThis();
    s_fiber_count++;
    m_id = s_fiber_id++;
    m_state = INIT;

    m_stackSize = stackSize ? stackSize : g_stack_size->getValue();
    m_stackPtr = (char*)Allocator::Alloc(m_stackSize);

    // assert(Fiber::GetThis().get() != this);
    m_ctx = boost::context::detail::make_fcontext(
        m_stackPtr + m_stackSize, m_stackSize, MainFun);


    // LANE_LOG_DEBUG(g_logger) << "Fiber::Fiber(...)id=" << m_id;
}
Fiber::Fiber() : m_defer(nullptr), m_panic(false), m_defer_panic(false) {
    s_fiber_count++;
    m_id = s_fiber_id++;
    m_state = INIT;
    m_stackSize = 0;
    m_stackPtr = nullptr;

    //空的，只是用来放main fiber
    // LANE_LOG_DEBUG(g_logger) << "Fiber::Fiber()id=" << m_id;
}
Fiber::~Fiber() {
    if (m_stackPtr) {
        LANE_ASSERT2(m_state == TERM || m_state == EXCE || m_state == INIT,
                     "m_state=" + std::to_string(m_state));
        Allocator::Dealloc(m_stackPtr, m_stackSize);
    }
    if (t_fiber.get() == this) {
        t_fiber.reset();
    }
    s_fiber_count--;
    // GetThis()->getId();
    // LANE_LOG_DEBUG(g_logger) << "Fiber::~Fiber()id=" << m_id;
    // Thread::GetName();
    // std::cout << "[raw log]\t" << __FILE__ << ":" << __LINE__
    //           << "\tFiber::~Fiber()id=" << m_id << std::endl;
}

void Fiber::reset(CallBackType cb) {
    LANE_ASSERT(m_stackPtr);
    LANE_ASSERT(m_state == TERM || m_state == EXCE || m_state == INIT);

    m_state = INIT;
    m_cb = cb;

    m_ctx = boost::context::detail::make_fcontext(
        m_stackPtr + m_stackSize, m_stackSize, MainFun);
}

void Fiber::swapIn() {
    LANE_ASSERT(m_state == INIT || m_state == HOLD || m_state == READY);
    if (m_withThread) {
        LANE_ASSERT(t_threadFiber == t_fiber && t_fiber->m_state == EXEC);
        SetThis(shared_from_this());
        t_threadFiber->m_state = HOLD;
        auto rt = boost::context::detail::jump_fcontext(m_ctx, this);
        m_ctx = rt.fctx;
        LANE_ASSERT(t_threadFiber == t_fiber && t_fiber->m_state == EXEC);

    } else {
        LANE_ASSERT(Scheduler::GetScheRunFiber() == t_fiber &&
                    t_fiber->m_state == EXEC);
        SetThis(shared_from_this());
        Scheduler::GetScheRunFiber()->m_state = HOLD;
        // LANE_LOG_DEBUG(g_logger) << "jump swpa in enter, m_ctx:" << m_ctx;
        auto rt = boost::context::detail::jump_fcontext(m_ctx, this);
        m_ctx = rt.fctx;
        // LANE_LOG_DEBUG(g_logger) << "jump swpa in exit";
        // LANE_LOG_DEBUG(g_logger) << "still alive after swapIn()?"
        //                          << "rt.fctx:" << rt.fctx;
        LANE_ASSERT(Scheduler::GetScheRunFiber() == t_fiber &&
                    (t_fiber->m_state == EXEC || t_fiber->m_state == EXCE));
    }

    // panic中设置m_state EXCE / m_panic = true / m_panic_str = XX
    if (m_state == EXCE) {
        // 跟 catch 中的流程一样，然后recovery可以获取这里的exection
        while (m_defer != nullptr) {
            m_defer->cb();
            m_defer = m_defer->next;
            // 如果cb中调用了recovery 会将m_panic
            // 置false，因此就可以恢复swapIn执行
            if (!m_panic) {
                // 是Excution 不是 Excetion
                m_state = READY;
                break;
            }
        }
        // 如果没有捕获，则抛出exection
        if (m_panic && !m_exections.empty()) {
            throw m_exections.front();
        }
    }

    LANE_ASSERT2(m_state == HOLD || m_state == READY || m_state == TERM ||
                     m_state == EXCE,
                 "m_state=" + std::to_string(m_state));
}
void Fiber::swapOut() {
    // m_state == HOLD || m_state == READY || m_state == TERM || m_state ==
    // EXCE
    LANE_ASSERT2(m_state != INIT, "m_state=" + std::to_string(m_state));
    if (m_withThread) {
        LANE_ASSERT(t_threadFiber != t_fiber &&
                    t_threadFiber->m_state == HOLD &&
                    t_fiber == shared_from_this());
        SetThis(t_threadFiber);
        boost::context::detail::jump_fcontext(t_threadFiber->m_ctx, nullptr);

        // LANE_ASSERT(t_threadFiber != t_fiber &&
        //             t_threadFiber->m_state == HOLD &&
        //             t_fiber == shared_from_this());
    } else {
        LANE_ASSERT(Scheduler::GetScheRunFiber() != t_fiber &&
                    Scheduler::GetScheRunFiber()->m_state == HOLD &&
                    t_fiber == shared_from_this());
        SetThis(Scheduler::GetScheRunFiber());
        // int rt = 0;
        // raw_ptr这一步不可省略
        // 临时对象在一行执行完才析构

        auto raw_ptr = Scheduler::GetScheRunFiber().get();
        LANE_ASSERT2(raw_ptr->m_ctx != nullptr, "jump into empty ptr")
        // LANE_LOG_DEBUG(g_logger)
        //     << "dead in swap out?"
        //     << " raw ptr:" << raw_ptr << "raw ptr m_ctx:" << raw_ptr->m_ctx;
        // LANE_LOG_DEBUG(g_logger) << "jump swpa out enter";
        boost::context::detail::jump_fcontext(raw_ptr->m_ctx, nullptr);
        // LANE_LOG_DEBUG(g_logger) << "jump swpa out exit";
        LANE_ASSERT(Scheduler::GetScheRunFiber() != t_fiber &&
                    Scheduler::GetScheRunFiber()->m_state == HOLD &&
                    t_fiber == shared_from_this());
    }
    LANE_ASSERT(m_state == EXEC);
}

void Fiber::MainFun(boost::context::detail::transfer_t in) {
    auto raw_ptr = GetThis().get();
    // 保存切换前（也就是线程主协程）的上下文
    auto from = reinterpret_cast<Fiber*>(in.data);
    if (from->m_withThread) {
        t_threadFiber->m_ctx = in.fctx;
        LANE_ASSERT(raw_ptr != t_threadFiber.get());
    } else {
        Scheduler::GetScheRunFiber()->m_ctx = in.fctx;
        LANE_ASSERT(raw_ptr != Scheduler::GetScheRunFiber().get());
    }


    try {
        raw_ptr->m_cb();
        raw_ptr->m_state = TERM;
    } catch (std::exception& e) {
        LANE_LOG_ERROR(g_logger) << "Fiber::MainFun() : exception:" << e.what();
        raw_ptr->m_state = EXCE;

        raw_ptr->m_panic = true;
        raw_ptr->m_exections.push_back(e);
        while (raw_ptr->m_defer != nullptr) {
            raw_ptr->m_defer->cb();
            raw_ptr->m_defer = raw_ptr->m_defer->next;
        }
        if (raw_ptr->m_panic) {
            throw e;
        }
    } catch (...) {
        LANE_LOG_ERROR(g_logger) << "Fiber::MainFun() : unknow exception。";
        raw_ptr->m_state = EXCE;
    }
    while (raw_ptr->m_defer != nullptr) {
        raw_ptr->m_defer->cb();
        raw_ptr->m_defer = raw_ptr->m_defer->next;
    }
    raw_ptr->swapOut();

    LANE_ASSERT2(false,
                 "never reach fiber_id=" + std::to_string(raw_ptr->getId()));
}
void Fiber::YieldToHold() {
    //因为swapOut的存在，函数不会正常结束，局部变量不会正常析构，此处就要避免创造第二个shared_ptr，影响fiber对象析构
    auto raw_ptr = GetThis().get();
    raw_ptr->m_state = HOLD;

    raw_ptr->swapOut();
}

void Fiber::YieldToReady() {
    auto raw_ptr = GetThis().get();
    raw_ptr->m_state = READY;

    raw_ptr->swapOut();
}

Fiber::ptr Fiber::GetThis() {
    if (t_fiber) {
        return t_fiber;
    }

    // 当在主线程中调用GetThis() 就初始化 t_fiber;
    LANE_ASSERT(t_threadFiber == nullptr);
    Fiber::ptr mainFiber(new Fiber());

    t_threadFiber = mainFiber;
    SetThis(mainFiber);
    return t_fiber;
}

Fiber::ptr Fiber::GetBefore() {
    return t_before;
}

void Fiber::SetThis(Fiber::ptr fiberPtr) {
    LANE_ASSERT(fiberPtr != nullptr &&
                (fiberPtr->m_state == INIT || fiberPtr->m_state == HOLD ||
                 fiberPtr->m_state == READY));
    t_fiber = fiberPtr;
    fiberPtr->m_state = EXEC;
}

void Fiber::_defer(std::function<void()> cb) {
    auto node = new struct Defer;
    node->cb = cb;
    node->next = m_defer;
    m_defer = node;
}

std::optional<std::exception> Fiber::_recovery() {
    if (GetBefore() && !GetBefore()->m_exections.empty()) {
        auto rt = m_exections.front();
        m_exections.pop_front();
        t_before->m_panic = false;
        return rt;
    }
    return std::nullopt;
}

void Fiber::_panic(std::exception e) {
    t_before = GetThis();
    auto raw_ptr = GetThis().get();
    raw_ptr->m_state = EXCE;
    raw_ptr->m_panic = true;
    raw_ptr->m_exections.push_back(e);
    raw_ptr->swapOut();
}

uint32_t Fiber::GetFiberId() {
    if (t_fiber != nullptr) {
        return t_fiber->getId();
    } else {
        return UINT32_MAX;
    }
}
}  // namespace lane