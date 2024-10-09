#include "base/mutex.h"

#include <sys/types.h>

#include <functional>

#include "base/fiber.h"
#include "base/iomanager.h"
#include "base/log.h"
#include "base/macro.h"
#include "base/timer.h"
namespace lane {
static Logger::ptr g_logger = LANE_LOG_NAME("system");
Semaphore::Semaphore(uint32_t count) {
    if (sem_init(&m_semaphore, 0, count)) {
        LANE_LOG_ERROR(g_logger)
            << "Semaphore::Semaphore(..) sem_init error : errno = " << errno
            << strerror(errno);
        throw std::logic_error("sem_init error");
    }
}
Semaphore::~Semaphore() {
    int rt = sem_destroy(&m_semaphore) == 0;
    LANE_ASSERT(rt);
}
void Semaphore::wait() {
    int rt = 0;
    while ((rt = sem_wait(&m_semaphore)) == EINTR)
        ;

    if (rt) {
        LANE_LOG_ERROR(g_logger)
            << "Semaphore::wait(..) sem_wait error : errno = " << errno
            << strerror(errno);
        throw std::logic_error("sem_wait error");
    }
}
bool Semaphore::waitForSeconds(time_t seconds) {
    struct timespec timeout;
    timeout.tv_sec = time(NULL) + seconds;  // 设置超时时间为当前时间加seconds秒
    timeout.tv_nsec = 0;

    int rt = sem_timedwait(&m_semaphore,
                           &timeout);  // 等待信号量，最多等待seconds秒

    return (rt == -1) && (errno == ETIMEDOUT);
}
void Semaphore::post() {
    if (sem_post(&m_semaphore)) {
        LANE_LOG_ERROR(g_logger)
            << "Semaphore::post(..) sem_post error : errno = " << errno
            << strerror(errno);
        throw std::logic_error("sem_post error");
    }
}

FiberMutex::~FiberMutex() {
    LANE_ASSERT(m_fs.getSem() == 1);
}

FiberSemaphore::FiberSemaphore(uint32_t count) : m_sem(count) {}
FiberSemaphore::~FiberSemaphore() {
    // LANE_ASSERT(m_sem == 0)
}

// 1、判断还有没有可用的资源
// 2、有就将资源减一并返回，没有就将当前协程放入等待队列，待资源被补充后唤醒
// 3、待解决的问题：如何让调度器感知，有协程在等待信号量，从而解决协程调度器忽略掉那些在等待信号量的协程而提前退出的bug
void FiberSemaphore::wait() {
    {
        Mutex::Lock lock(m_mutex);
        --m_sem;
        LANE_LOG_DEBUG(g_logger) << "m_sem--" << m_sem;
        if (m_sem >= 0) {  // 有资源
            return;
        } else {
            IOManager* iom = IOManager::GetThis();
            iom->addBlock();
            m_waitQueue.emplace(iom, Fiber::GetThis());
        }
    }
    Fiber::YieldToHold();  // 协程转变成Hold状态，等待资源
}

void FiberSemaphore::wait(Mutex& m_mutex) {


    --m_sem;
    LANE_LOG_DEBUG(g_logger) << "m_sem--" << m_sem;
    if (m_sem >= 0) {  // 有资源
        return;
    } else {
        IOManager* iom = IOManager::GetThis();
        iom->addBlock();
        m_waitQueue.emplace(iom, Fiber::GetThis());
    }
    m_mutex.unlock();
    Fiber::YieldToHold();  // 协程转变成Hold状态，等待资源
    m_mutex.lock();
}

// 这里操作逻辑和wait类似，但是唤醒的方式有两种，一是通过其他协程调用post、二是通过定时器超时的回调函数。
// 这里将seconds定为秒数吧
bool FiberSemaphore::waitForSeconds(time_t seconds) {
    Mutex::Lock lock(m_mutex);

    if (m_sem > 0) {  // 有资源
        m_sem -= 1;
        return false;
    } else {                      // 无资源
        LANE_ASSERT(m_sem == 0);  // 不可小于0

        std::shared_ptr<int> tinfo(new int(0));  // 超时信息
        Timer::ptr           timer;
        IOManager*           iom = IOManager::GetThis();
        Fiber::ptr           fiber = lane::Fiber::GetThis();

        if (seconds != -1) {
            LANE_ASSERT(seconds >= 0)
            std::weak_ptr<int> winfo(tinfo);

            timer = iom->addTimer(seconds * 1000, [winfo, iom, fiber]() {
                auto t = winfo.lock();
                if (!t || *t) {
                    return;
                }
                *t = ETIMEDOUT;
                iom->addTask(fiber);
            });
        }
        iom->addBlock();
        m_waitQueue.push(std::make_pair(iom, fiber));

        // 这里在退出前，记得释放锁，不然就死锁了
        lock.unlock();
        Fiber::YieldToHold();  // 协程转变成Hold状态，等待资源
        lock.lock();

        // 目前的实现是不需要再加锁的。
        // 回来要检查是因为什么被唤醒的。
        if (timer) {  // 添加过定时器
            timer->cancel();
        }
        if (*tinfo) {  // 非零即超时
            LANE_ASSERT(*tinfo == ETIMEDOUT)
            return true;
        }
    }

    LANE_ASSERT(m_sem > 0)
    m_sem -= 1;
    return false;
}
void FiberSemaphore::post() {
    Mutex::Lock lock(m_mutex);
    // LANE_ASSERT(m_sem >= 0);
    m_sem++;
    LANE_LOG_DEBUG(g_logger) << "m_sem++" << m_sem;
    if (m_waitQueue.empty() == false) {
        auto toWakeup = m_waitQueue.front();

        m_waitQueue.pop();
        toWakeup.first->addTask(toWakeup.second);
        toWakeup.first->delBlock();
    }

    return;
}

void FiberSemaphore::post(Mutex& m_mutex) {
    // LANE_ASSERT(m_sem >= 0);
    m_sem++;

    if (m_waitQueue.empty() == false) {
        auto toWakeup = m_waitQueue.front();

        m_waitQueue.pop();
        toWakeup.first->addTask(toWakeup.second);
        toWakeup.first->delBlock();
    }

    return;
}

int32_t FiberSemaphore::getSem() {
    Mutex::Lock lock(m_mutex);
    return m_sem;
}

void FiberSemaphore::reset() {
    Mutex::Lock lock(m_mutex);
    m_sem = 0;
    while (!m_waitQueue.empty()) {  //全部唤醒
        auto toWakeup = m_waitQueue.front();

        m_waitQueue.pop();
        toWakeup.first->addTask(toWakeup.second);
        toWakeup.first->delBlock();
    }
}

void FiberSemaphore::resize(int32_t size) {

    Mutex::Lock lock(m_mutex);
    m_sem = size;
    while (!m_waitQueue.empty()) {  //全部唤醒
        auto toWakeup = m_waitQueue.front();

        m_waitQueue.pop();
        toWakeup.first->addTask(toWakeup.second);
        toWakeup.first->delBlock();
    }
}


// FiberCondition::FiberCondition() : m_sem(0) {}
// FiberCondition::~FiberCondition() {}

// void FiberCondition::wait(ScopedLockImpl<FiberMutex>& Lock,
//                           std::function<bool(void)>   cb) {
//     m_cb = cb;
//     while (true) {
//         {  // lock
//             if (cb()) {
//                 break;
//             } else {
//                 m_waitQueue.emplace(IOManager::GetThis(), Fiber::GetThis());
//                 Lock.unlock();
//                 Fiber::YieldToHold();
//             }
//         }
//     }
// }
// bool FiberCondition::notify() {

// }
// bool FiberCondition::notify_all() {}
}  // namespace lane