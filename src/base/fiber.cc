#include "base/fiber.h"

#include <boost/context/detail/fcontext.hpp>
#include <boost/context/detail/index_sequence.hpp>
#include <cstddef>

#include "base/config.h"
#include "base/scheduler.h"
namespace lane {
static Logger::ptr           g_logger = LANE_LOG_NAME("system");
static std::atomic<uint32_t> s_fiber_id = {0};
static std::atomic<uint32_t> s_fiber_count = {0};
// 切缘到线程的协程
static thread_local Fiber::ptr t_threadFiber(nullptr);
// 当前执行的协程
static thread_local Fiber::ptr t_fiber(nullptr);

static ConfigVar<uint32_t>::ptr g_stack_size =
    ConfigVarMgr::GetInstance()->lookUp("fiber.stackSize",
                                        uint32_t(1024 * 8),
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
    : m_cb(cb), m_withThread(withThread) {
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
Fiber::Fiber() {
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
        LANE_ASSERT(m_state == TERM || m_state == EXCE || m_state == INIT);
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
        auto rt = boost::context::detail::jump_fcontext(m_ctx, this);
        m_ctx = rt.fctx;
        LANE_ASSERT(Scheduler::GetScheRunFiber() == t_fiber &&
                    t_fiber->m_state == EXEC);
    }

    LANE_ASSERT(m_state == HOLD || m_state == READY || m_state == TERM ||
                m_state == EXCE);
}
void Fiber::swapOut() {
    // m_state == HOLD || m_state == READY || m_state == TERM || m_state ==
    // EXCE
    LANE_ASSERT(m_state != EXEC && m_state != INIT);
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
        boost::context::detail::jump_fcontext(raw_ptr->m_ctx, nullptr);

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
    } catch (...) {
        LANE_LOG_ERROR(g_logger) << "Fiber::MainFun() : unknow exception。";
        raw_ptr->m_state = EXCE;
    }

    raw_ptr->swapOut();

    LANE_ASSERT2(false,
                 "never reach fiber_id=" + std::to_string(raw_ptr->getId()));
}
void Fiber::YieldToHold() {
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
void Fiber::SetThis(Fiber::ptr fiberPtr) {
    LANE_ASSERT(fiberPtr != nullptr &&
                (fiberPtr->m_state == INIT || fiberPtr->m_state == HOLD ||
                 fiberPtr->m_state == READY));
    t_fiber = fiberPtr;
    fiberPtr->m_state = EXEC;
}

uint32_t Fiber::GetFiberId() {
    if (t_fiber != nullptr) {
        return t_fiber->getId();
    } else {
        return UINT32_MAX;
    }
}
}  // namespace lane