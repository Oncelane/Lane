#include "base/timer.h"

#include "base/macro.h"
namespace lane {

    // 注意加锁！！！！！！！！

    bool Timer::Comparator::operator()(const Timer::ptr& lhs,
                                       const Timer::ptr& rhs) const {
        if (!lhs) {
            return true;
        }

        if (!rhs) {
            return false;
        }
        if (lhs->m_next < rhs->m_next) {
            return true;
        } else if (lhs->m_next > rhs->m_next) {
            return false;
        } else {
            // 使用地址作比较，保证set内顺序的一致性
            return lhs.get() < rhs.get();
        }
    }
    Timer::Timer(uint64_t interval,
                 bool isLoop,
                 CallBackType cb,
                 TimerManager* mgr)
        : m_interval(interval),
          m_isLoop(isLoop),
          m_cb(cb),
          m_mgr(mgr) {
        uint64_t now = lane::GetCurrentMiliSTime();

        m_next = now + m_interval;
    }
    Timer::Timer(uint64_t next) {
        m_next = next;
    }

    void Timer::cancel() {
        // 读写锁可以连用吗？？？
        LANE_ASSERT(m_mgr != nullptr);
        TimerManager::MutexType::WriteLock wlock(m_mgr->m_rwmutex);
        // 直接使用shared from this 智能指针来在时间堆中查找并取消
        auto pos = m_mgr->m_tss.find(shared_from_this());
        if (pos == m_mgr->m_tss.end()) {
            return;
        }
        m_mgr->m_tss.erase(pos);
        wlock.unlock();

        m_cb = nullptr;
    }

    // 刷新定时器
    void Timer::refresh() {
        LANE_ASSERT(m_mgr != nullptr);
        TimerManager::MutexType::WriteLock wlock(m_mgr->m_rwmutex);
        // 直接使用shared from this 智能指针来在时间堆中查找并取消
        auto pos = m_mgr->m_tss.find(shared_from_this());
        if (pos == m_mgr->m_tss.end()) {
            return;
        }

        m_mgr->m_tss.erase(pos);
        wlock.unlock();
        // 从堆中删除后再重新加入，利用set insert的自动排序
        if (m_cb != nullptr) {
            m_next = lane::GetCurrentMiliSTime() + m_interval;
            m_mgr->addTimer(shared_from_this());
        }
    }

    // 更改定时器的时间间隔
    bool Timer::reset(uint64_t intervl, bool fromNow) {
        LANE_ASSERT(m_mgr != nullptr);
        TimerManager::MutexType::WriteLock wlock(m_mgr->m_rwmutex);
        auto pos = m_mgr->m_tss.find(shared_from_this());
        if (pos == m_mgr->m_tss.end()) {
            return false;
        }

        m_mgr->m_tss.erase(pos);
        wlock.unlock();

        uint64_t start;

        if (m_cb == nullptr) {
            LANE_ASSERT(false);
            return false;
        }

        if (fromNow) {
            start = lane::GetCurrentMiliSTime();
        } else {
            start = m_next - m_interval;
        }

        m_next = start + intervl;
        m_interval = intervl;

        m_mgr->addTimer(shared_from_this());
        return true;
    }

    // 向时间堆中添加定时器
    Timer::ptr TimerManager::addTimer(uint64_t interval,
                                      Timer::CallBackType cb,
                                      bool isLoop) {
        Timer::ptr time(new Timer(interval, isLoop, cb, this));

        if (addTimer(time)) {
            return time;
        } else {
            return nullptr;
        }

        return nullptr;
    }
    bool TimerManager::addTimer(Timer::ptr time) {
        LANE_ASSERT(time->m_cb != nullptr);

        MutexType::WriteLock wlock(m_rwmutex);
        auto pos = m_tss.insert(time).first;
        // 插到前面需要tickle;
        if (pos == m_tss.begin()) {
            wlock.unlock();
            // 唤醒等待旧定时器的线程
            onTimerInsertedFront();
        }

        return true;
    }

    void TimerManager::listAllExpire(
        std::vector<std::function<void(void)>>& cbs) {
        Timer::ptr now_t(new Timer(lane::GetCurrentMiliSTime()));
        // 不能采用加读锁后加写锁的方式，关读锁后到加写锁的过程中，可能会导致不一致，出现线程安全问题。
        MutexType::WriteLock wlock(m_rwmutex);
        // upper_bound在时间堆中查找超时的定时事件
        // target是一个新建立的空timer，时间为now()
        auto end = m_tss.upper_bound(now_t);
        std::vector<Timer::ptr> expired(m_tss.begin(), end);

        cbs.reserve(expired.size());

        m_tss.erase(m_tss.begin(), end);

        for (auto itr = expired.begin(); itr != expired.end(); itr++) {
            LANE_ASSERT((*itr)->m_cb != nullptr);
            cbs.push_back((*itr)->m_cb);
            if ((*itr)->m_isLoop) {
                (*itr)->m_next =
                    (*itr)->m_interval +
                    now_t->m_next;  // 重新加入需要循环触发的timer时间
                m_tss.insert(*itr);
            } else {
                (*itr)->m_cb =
                    nullptr;  // timer结构体中，只取出callback，剩余的信息不需要
                //(*itr)->m_next = ~0ull;
            }
        }
    }

    uint64_t TimerManager::getFirstTimeOut() {
        MutexType::ReadLock rlock(m_rwmutex);
        //
        if (m_tss.begin() == m_tss.end()) {
            return ~0ull;
        }
        return (*m_tss.begin())->m_next;
    }

    bool TimerManager::hasTimer() {
        MutexType::ReadLock rlock(m_rwmutex);
        return !m_tss.empty();
    }

}  // namespace lane