/*******************************************
 * Author : Lane
 * Email: : 1981811204@qq.com
 * CreateTime : 2023-02-16 15:50
 * LastModified : 2023-02-16 15:50
 * Filename : timer.h
 * Description : 1.0 *
 ************************************/

#ifndef __LANE_TIMER_H__
#define __LANE_TIMER_H__
#include <dirent.h>
#include <stdint.h>
#include <sys/types.h>

#include <algorithm>
#include <functional>
#include <memory>
#include <set>
#include <vector>

#include "base/mutex.h"
#include "base/util.h"
namespace lane {
    class TimerManager;
    class Timer : public std::enable_shared_from_this<Timer> {
        friend class TimerManager;

       public:
        typedef std::shared_ptr<Timer> ptr;
        typedef std::function<void(void)> CallBackType;
        struct Comparator {
            bool operator()(const Timer::ptr& lhs, const Timer::ptr& rhs) const;
        };

       public:
        Timer(uint64_t interval,
              bool isLoop,
              CallBackType cb,
              TimerManager* mgr);
        Timer(uint64_t next);

        // 时间事件的取消，刷新，重置用事件的接口实现而不是时间堆，省了点参数
        void cancel();
        void refresh();
        bool reset(uint64_t itl, bool fromNow);

       private:
        // 间隔
        uint64_t m_interval = 0;
        // 下一次执行时间;
        uint64_t m_next = ~0ull;
        // 是否循环
        bool m_isLoop = false;
        CallBackType m_cb = nullptr;
        TimerManager* m_mgr = nullptr;  // 防止循环引用。
    };

    class TimerManager {
        friend class Timer;

       public:
        // typedef std::shared_ptr<TimerManager> ptr;
        typedef std::set<Timer::ptr, Timer::Comparator> TimerSetType;
        typedef RWMutex MutexType;

       public:
        // interval：毫秒数
        Timer::ptr addTimer(uint64_t interval,
                            Timer::CallBackType cb,
                            bool isLoop = false);
        bool addTimer(Timer::ptr time);

        // 获得所有超时事件，由epoll_wait驱动，epoll_wait在tcpserver中是有主从reactor模型的
        // 主reactor从定时器中拿到事件后应该分配给子reactor执行，否则会因为事件处理导致epoll_wait不能及时执行
        void listAllExpire(std::vector<std::function<void(void)>>& cbs);
        uint64_t getFirstTimeOut();
        bool hasTimer();

       public:
        virtual void onTimerInsertedFront() = 0;
        virtual ~TimerManager() {}

       private:
        TimerSetType m_tss;
        MutexType m_rwmutex;
    };
}  // namespace lane

#endif