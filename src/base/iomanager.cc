#include "base/iomanager.h"

#include <algorithm>
#include <cstddef>
#include <random>

#include "base/log.h"
#include "base/macro.h"
#include "base/timer.h"
namespace lane {

static Logger::ptr g_logger = LANE_LOG_NAME("system");
void               IOManager::FdContext::EventContext::reset() {
    m_fiber.reset();
    m_cb = nullptr;
    m_scheduler = nullptr;
}

void IOManager::FdContext::TrigleEvent(Event event) {
    if (!(m_events & event)) {
        LANE_LOG_ERROR(g_logger)
            << "FdContext::TrigleEvent(event=" << event << "),"
            << "but m_event & event == 0, don't listen this event!";
        return;
    }
    m_events = (Event)(m_events & (~event));
    EventContext &ectx = getEventContext(event);
    LANE_ASSERT((ectx.m_fiber || ectx.m_cb) && (ectx.m_scheduler != nullptr));

    if (ectx.m_fiber) {
        ectx.m_scheduler->schedule(&ectx.m_fiber);
    } else {
        ectx.m_scheduler->schedule(&ectx.m_cb);
    }

    resetEventContext(event);
}

IOManager::FdContext::EventContext &IOManager::FdContext::getEventContext(
    Event event) {
    switch (event) {
        case READ:
            return read;
            break;
        case WRITE:
            return write;
            break;
        default:
            break;
    }
    LANE_ASSERT2(false, "unknow Event");
}

void IOManager::FdContext::resetEventContext(Event event) {
    switch (event) {
        case READ:
            read.reset();
            return;
        case WRITE:
            write.reset();
            return;
        default:
            break;
    }
    LANE_ASSERT2(false, "unknow Event");
}

IOManager::IOManager(uint32_t threadCount, const std::string &name, bool useCur)
    : Scheduler(threadCount, name, useCur), m_re(m_rd()) {
    // 将pipfd读端设置成非阻塞。
    int flag = 0;
    int rt = 0;
    rt = pipe(m_pipfd);
    LANE_ASSERT(rt == 0);
    flag = fcntl(m_pipfd[0], F_GETFL);
    rt = fcntl(m_pipfd[0], F_SETFL, flag | O_NONBLOCK);
    LANE_ASSERT(rt == 0);

    // fd可读/边沿触发
    m_epollfd = epoll_create(50);
    epoll_event epevent = {0};
    epevent.events = EPOLLIN | EPOLLET;
    epevent.data.fd = m_pipfd[0];
    rt = epoll_ctl(m_epollfd, EPOLL_CTL_ADD, m_pipfd[0], &epevent);

    LANE_ASSERT(rt == 0);
    resize(32);

    start();
}

IOManager::~IOManager() {
    stop();
    close(m_pipfd[0]);
    close(m_pipfd[1]);
    close(m_epollfd);
    for (size_t i = 0; i < m_fdContexts.size(); i++) {
        delete m_fdContexts[i];
    }
}
void IOManager::resize(size_t size) {
    MutexType::WriteLock wrlock(m_mutex);
    if (size <= m_fdContexts.size()) {
        // 不可减少
        return;
    }
    m_fdContexts.resize(size);
    for (size_t i = 0; i < size; i++) {
        if (m_fdContexts[i] == nullptr) {
            m_fdContexts[i] = new FdContext;
            m_fdContexts[i]->m_fd = i;
        }
    }
}

int IOManager::addEvent(int fd, Event event, std::function<void(void)> cb) {
    MutexType::ReadLock rdlock(m_mutex);
    if ((int)m_fdContexts.size() <= fd) {
        rdlock.unlock();
        resize(fd * 1.5);
        rdlock.lock();
    }
    FdContext *fdctx = m_fdContexts[fd];
    rdlock.unlock();

    FdContext::MutexType::Lock lock(fdctx->m_mutex);
    if (fdctx->m_events & event) {
        // 事件存在
        LANE_LOG_ERROR(g_logger)
            << "fd = " << fd << ", event = " << event << " many time add";
        LANE_ASSERT(false);
    }

    int         op = fdctx->m_events ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;
    epoll_event epevent = {0};
    epevent.events = fdctx->m_events | event | EPOLLET;
    epevent.data.ptr = fdctx;

    int rt = epoll_ctl(m_epollfd, op, fd, &epevent);
    if (rt) {
        LANE_LOG_ERROR(g_logger)
            << "epoll_ctl(" << m_epollfd << ", " << op << ", " << fd << ", "
            << epevent.events << "):"
            << "<errno[" << errno << "]:" << strerror(errno) << ">";

        return -1;
    }
    LANE_LOG_DEBUG(g_logger) << "m_penddingEventCount++";
    m_penddingEventCount++;
    fdctx->m_events = (Event)(fdctx->m_events | event);
    FdContext::EventContext &ectx = fdctx->getEventContext(event);

    LANE_ASSERT(!ectx.m_cb && !ectx.m_fiber && !ectx.m_scheduler);

    if (cb) {
        ectx.m_cb = cb;
        // error note: 不能用shared_from_this
        // ectx.m_scheduler = Scheduler::GetThis() || shared_from_this();
    } else {
        ectx.m_fiber = Fiber::GetThis();
    }
    ectx.m_scheduler = Scheduler::GetThis() ? Scheduler::GetThis() : this;

    return 0;
}

int IOManager::delEvent(int fd, Event event) {
    MutexType::ReadLock rdlock(m_mutex);
    if ((int)m_fdContexts.size() <= fd) {
        // fd过大
        return -1;
    }
    FdContext *fdctx = m_fdContexts[fd];
    rdlock.unlock();

    FdContext::MutexType::Lock lock(fdctx->m_mutex);
    if (!(fdctx->m_events & event)) {
        // 事件不存在
        return -1;
    }
    Event       leftEvent = (Event)(fdctx->m_events & (~event));
    int         op = leftEvent ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
    epoll_event epevent = {0};
    epevent.events = leftEvent | EPOLLET;
    epevent.data.ptr = fdctx;

    int rt = epoll_ctl(m_epollfd, op, fd, &epevent);
    if (rt) {
        LANE_LOG_ERROR(g_logger)
            << "epoll_ctl(" << m_epollfd << ", " << op << ", " << fd << ", "
            << epevent.events << "):"
            << "<errno[" << errno << "]:" << strerror(errno) << ">";

        return -1;
    }
    LANE_LOG_DEBUG(g_logger) << "m_penddingEventCount--";
    m_penddingEventCount--;
    fdctx->m_events = leftEvent;
    fdctx->resetEventContext(event);
    return 0;
}

int IOManager::cancelEvent(int fd, Event event) {
    MutexType::ReadLock rdlock(m_mutex);
    if ((int)m_fdContexts.size() <= fd) {
        // fd过大
        return -1;
    }
    FdContext *fdctx = m_fdContexts[fd];
    rdlock.unlock();

    FdContext::MutexType::Lock lock(fdctx->m_mutex);
    if (!(fdctx->m_events & event)) {
        // 事件不存在
        return -1;
    }
    Event       leftEvent = (Event)(fdctx->m_events & (~event));
    int         op = leftEvent ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
    epoll_event epevent = {0};
    epevent.events = leftEvent | EPOLLET;
    epevent.data.ptr = fdctx;

    int rt = epoll_ctl(m_epollfd, op, fd, &epevent);
    if (rt) {
        LANE_LOG_ERROR(g_logger)
            << "epoll_ctl(" << m_epollfd << ", " << op << ", " << fd << ", "
            << epevent.events << "):"
            << "<errno[" << errno << "]:" << strerror(errno) << ">";

        return -1;
    }
    fdctx->TrigleEvent(event);
    LANE_LOG_DEBUG(g_logger) << "m_penddingEventCount--";
    m_penddingEventCount--;
    return 0;
}

int IOManager::cancelAll(int fd) {
    MutexType::ReadLock rdlock(m_mutex);
    if ((int)m_fdContexts.size() <= fd) {
        // fd过大
        return -1;
    }
    FdContext *fdctx = m_fdContexts[fd];
    rdlock.unlock();

    FdContext::MutexType::Lock lock(fdctx->m_mutex);
    if (fdctx->m_events == NONE) {
        // 没有一个事件
        return -1;
    }
    Event       hasEvent = (Event)(fdctx->m_events);
    int         op = EPOLL_CTL_DEL;
    epoll_event epevent = {0};
    epevent.events = NONE;
    epevent.data.ptr = fdctx;

    int rt = epoll_ctl(m_epollfd, op, fd, &epevent);
    if (rt) {
        LANE_LOG_ERROR(g_logger)
            << "epoll_ctl(" << m_epollfd << ", " << op << ", " << fd << ", "
            << epevent.events << "):"
            << "<errno[" << errno << "]:" << strerror(errno) << ">";

        return -1;
    }
    if (hasEvent & READ) {
        fdctx->TrigleEvent(READ);
        LANE_LOG_DEBUG(g_logger) << "m_penddingEventCount--";
        m_penddingEventCount--;
    }

    if (hasEvent & WRITE) {
        fdctx->TrigleEvent(WRITE);
        LANE_LOG_DEBUG(g_logger) << "m_penddingEventCount--";
        m_penddingEventCount--;
    }
    LANE_ASSERT(fdctx->m_events == NONE);
    return 0;
}

int IOManager::getfdEvent(int fd) {
    MutexType::ReadLock rdlock(m_mutex);
    if ((int)m_fdContexts.size() <= fd) {
        // fd过大
        return -1;
    }
    FdContext *fdctx = m_fdContexts[fd];
    rdlock.unlock();
    return fdctx->m_events;
}

bool IOManager::isStoped() {
    return Scheduler::isStoped() && m_penddingEventCount == 0 &&
           !(TimerManager::hasTimer());
}

static const uint32_t MAXFD = 256;

static uint64_t MAX_TIMEOUT = 3000;

void IOManager::idle() {
    epoll_event *                epevents = new epoll_event[MAXFD]();
    std::shared_ptr<epoll_event> shared_event(
        epevents, [](epoll_event *ptr) { delete[] ptr; });
    while (!isStoped()) {
        int rt = 0;
        do {
            uint64_t curTime = lane::GetCurrentMiliSTime();
            uint64_t firstTimerTime = TimerManager::getFirstTimeOut();
            uint64_t timeOut =
                firstTimerTime > curTime ? firstTimerTime - curTime : 0;

            timeOut = std::min(timeOut, MAX_TIMEOUT);
            rt = epoll_wait(m_epollfd, epevents, MAXFD, (time_t)timeOut);
        } while ((rt < 0) && (errno == EINTR));

        std::vector<Timer::CallBackType> cbs;
        TimerManager::listAllExpire(cbs);

        Scheduler::schedule(cbs.begin(), cbs.end());

        for (int i = 0; i < rt; i++) {
            // int i = newOrder[index];
            // pip IO
            if (epevents[i].data.fd == m_pipfd[0]) {
                // 唤醒就行，无需加锁
                char pipbuf[256];
                LANE_LOG_DEBUG(g_logger) << "tickle";
                while (read(m_pipfd[0], pipbuf, 256) > 0)
                    ;

                continue;
            }
            // else
            // socket IO
            FdContext *fdctx = (FdContext *)(epevents[i].data.ptr);
            // modify...
            Event triEvent = NONE;

            // LANE_LOG_FATAL(g_logger)
            //     << "lock on i=" << i << " fd=" << fdctx->m_fd;

            FdContext::MutexType::Lock lock(fdctx->m_mutex);

            // sleep_f(1);
            // (EPOLLIN | EPOLLOUT) 整体可以作为一个叫 IOMASK的宏。
            if (epevents[i].events & (EPOLLHUP | EPOLLERR)) {
                epevents[i].events |= ((EPOLLIN | EPOLLOUT) & fdctx->m_events);
            }
            // 以fdctx关心的事件作为掩码，从epevents中萃取事件。
            triEvent = (Event)(((READ | WRITE) & fdctx->m_events) &
                               epevents[i].events);
            /*
             * 这里加锁后再次判断是必要的，因为可能存在在加锁前，同一个fd多次触发事件，
             * 导致多个线程epoll_wait到同一fd，出现多线程操作同一fd的局面。
             */
            if ((fdctx->m_events & triEvent) == NONE) {
                continue;
            }

            int         fd = fdctx->m_fd;
            Event       leftEvent = (Event)(fdctx->m_events & (~triEvent));
            int         op = leftEvent ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
            epoll_event epe = {0};
            epe.events = leftEvent | EPOLLET;
            epe.data.ptr = fdctx;

            int rteCtl = epoll_ctl(m_epollfd, op, fd, &epe);
            if (rteCtl) {
                // 修改m_epollfd set失败，下次还会触发，所以continue就好了
                LANE_LOG_ERROR(g_logger)
                    << "epoll_ctl(" << m_epollfd << ", " << op << ", " << fd
                    << ", " << epe.events << "):"
                    << "<errno[" << errno << "]:" << strerror(errno) << ">";
                continue;
            }

            if (triEvent & READ) {
                fdctx->TrigleEvent(READ);
                LANE_LOG_DEBUG(g_logger) << "m_penddingEventCount--";
                m_penddingEventCount--;
            }

            if (triEvent & WRITE) {
                fdctx->TrigleEvent(WRITE);
                LANE_LOG_DEBUG(g_logger) << "m_penddingEventCount--";
                m_penddingEventCount--;
            }
            // for debug 确保m_penddingEventCount符合预期。
            LANE_ASSERT((int)m_penddingEventCount.load() >= 0);
        }

        Fiber::YieldToHold();
    }
}
void IOManager::tickle() {
    if (!HasIdleThread())
        return;
    int rt = write(m_pipfd[1], "T", 1);
    LANE_ASSERT(rt == 1);
    // LANE_LOG_DEBUG(g_logger) << "tickle" << rt << strerror(errno) <<
    // m_pipfd[1];
}
void IOManager::onTimerInsertedFront() {
    tickle();
}
IOManager *IOManager::GetThis() {
    return dynamic_cast<IOManager *>(Scheduler::GetThis());
}
}  // namespace lane