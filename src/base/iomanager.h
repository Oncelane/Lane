/*******************************************
 * Author : Lane
 * Email: : 1657015850@qq.com
 * CreateTime : 2023-02-05 16:30
 * LastModified : 2023-02-05 16:30
 * Filename : iomanager.h
 * Description : 1.0 *
 ************************************/

#ifndef __LANE_IOMANAGER_H__
#define __LANE_IOMANAGER_H__

#include <fcntl.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <random>
#include <vector>

#include "base/fiber.h"
#include "base/log.h"
#include "base/mutex.h"
#include "base/scheduler.h"
#include "base/timer.h"
namespace lane {

class IOManager : public Scheduler, public TimerManager {
public:
    typedef RWMutex                    MutexType;
    typedef std::shared_ptr<IOManager> ptr;
    enum Event { NONE = 0, READ = EPOLLIN, WRITE = EPOLLOUT };
    struct FdContext {
        using MutexType = SpinMutex;
        struct EventContext {
            void                      reset();
            Fiber::ptr                m_fiber = nullptr;
            std::function<void(void)> m_cb = nullptr;
            // 记录协程调度器，表示，当事件发生，fiber or
            // callback应该使用哪个协程调度器调度。
            Scheduler* m_scheduler = nullptr;
        };
        // nolock
        void          TrigleEvent(Event event);
        EventContext& getEventContext(Event event);
        void          resetEventContext(Event event);
        EventContext  read;         // 读事件Handle
        EventContext  write;        // 写事件Handle
        Event     m_events = NONE;  // 记录当前FdContext哪些事件有效
        int       m_fd = -1;
        MutexType m_mutex;
    };

public:
    IOManager(uint32_t           threadCount = 1,
              const std::string& name = "",
              bool               useCur = true);
    virtual ~IOManager() override;

public:
    void resize(size_t size);
    int  addEvent(int fd, Event event, std::function<void(void)> cb = nullptr);
    int  delEvent(int fd, Event event);
    int  cancelEvent(int fd, Event event);
    int  cancelAll(int fd);
    int  getfdEvent(int fd);
    void addBlock() {
        m_blockFiber++;
    }
    void delBlock() {
        m_blockFiber--;
    }

public:
    virtual bool isStoped() override;
    virtual void idle() override;
    virtual void tickle() override;
    virtual void onTimerInsertedFront() override;

public:
    static IOManager* GetThis();

private:
    std::vector<FdContext*>    m_fdContexts;
    std::atomic<uint32_t>      m_penddingEventCount = {0};
    int                        m_pipfd[2];
    int                        m_epollfd;
    MutexType                  m_mutex;
    std::random_device         m_rd;
    std::default_random_engine m_re;
    std::atomic<int32_t>       m_blockFiber = {0};
};
}  // namespace lane

#endif