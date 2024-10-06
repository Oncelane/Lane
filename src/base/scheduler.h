/*******************************************
 * Author : Lane
 * Email: : 1657015850@qq.com
 * CreateTime : 2023-02-02 09:37
 * LastModified : 2023-02-02 09:37
 * Filename : scheduler.h
 * Description : 1.0 *
 ************************************/

#ifndef __LANE_SCHEDULER_H__

#define __LANE_SCHEDULER_H__
#include <atomic>
#include <functional>
#include <list>
#include <memory>
#include <queue>
#include <unordered_map>
#include <vector>

#include "base/config.h"
#include "base/fiber.h"
#include "base/fiberAndThread.h"
#include "base/log.h"
#include "base/mutex.h"
#include "base/thread.h"
#include "base/threadSavequeue.h"
#include "base/util.h"
#include "base/workStealQueue.h"
namespace lane {

static ConfigVar<size_t>::ptr s_subQueueMaxSize =
    ConfigVarMgr::GetInstance()->lookUp("schedule.subQueueQueueMax",
                                        size_t(32),
                                        "max size of RoutineQueue");
// 类线程池

class Scheduler {
public:
    typedef std::shared_ptr<Scheduler> ptr;
    typedef Mutex                      MutexType;

public:
    Scheduler(uint32_t           threadCount,
              const std::string& name,
              bool               useCur = false);
    virtual ~Scheduler();
    const std::string& getName() const {
        return m_name;
    }
    void start();
    void stop();
    void run();
    bool HasIdleThread() const {
        return m_idleThreadCount != 0;
    };

private:
    inline std::vector<FiberAndThread> takeTask(pid_t tId);

public:
    virtual bool isStoped();
    virtual void idle();
    virtual void tickle();

public:
    static Scheduler* GetThis();

    static Fiber::ptr GetScheRunFiber();
    std::ostream&     dump(std::ostream& os);

public:
    template <typename FiberOrCb>
    void schedule(FiberOrCb fOrcb,
                  pid_t     tId = Thread::GetTid(),
                  bool      force = false) {
        bool needTickle = false;
        {
            // MutexType::Lock lock(m_mutex);

            needTickle = scheduleNoLock(fOrcb, tId, force);
        }

        if (needTickle) {
            tickle();
        }
    }

    template <typename ForwardIterator>
    void schedule(ForwardIterator begin,
                  ForwardIterator end,
                  pid_t           tId = Thread::GetTid(),
                  bool            force = false) {
        bool needTickle = false;
        {
            // MutexType::Lock lock(m_mutex);

            for (ForwardIterator itr = begin; itr != end; itr++) {
                needTickle = scheduleNoLock(*itr, tId, force) || needTickle;
            }
        }

        if (needTickle) {
            tickle();
        }
    }

    template <typename FiberOrCb>
    void addTask(FiberOrCb fOrcb,
                 pid_t     tId = Thread::GetTid(),
                 bool      force = false) {
        bool needTickle = false;
        {
            // MutexType::Lock lock(m_mutex);

            needTickle = scheduleNoLockFirst(fOrcb, tId, force);
        }

        if (needTickle) {
            tickle();
        }
    }

    template <typename ForwardIterator>
    void addTask(ForwardIterator begin,
                 ForwardIterator end,
                 pid_t           tId = Thread::GetTid(),
                 bool            force = false) {
        bool needTickle = false;
        {
            // MutexType::Lock lock(m_mutex);
            for (ForwardIterator itr = begin; itr != end; itr++) {
                needTickle =
                    scheduleNoLockFirst(*itr, tId, force) || needTickle;
            }
        }

        if (needTickle) {
            tickle();
        }
    }

protected:
    template <typename FiberOrCb>
    bool scheduleNoLockFirst(FiberOrCb fOrCb, pid_t tId, bool force) {
        bool           needTickle = m_mainQueue.empty();
        FiberAndThread fiC(fOrCb, force ? tId : -1);
        if (fiC.m_cb || fiC.m_fiber) {
            // printf("%d",m_subQueuesmap[tId]);
            m_mainQueue.push(fiC);
        }
        return needTickle;
    }

    template <typename FiberOrCb>
    bool scheduleNoLock(FiberOrCb fOrCb, pid_t tId, bool force) {
        // static int localcount = 0;
        // static int maincount = 0;
        bool           needTickle = m_mainQueue.empty();
        FiberAndThread fiC(fOrCb, force ? tId : -1);
        if (fiC.m_cb || fiC.m_fiber) {
            // printf("%d",m_subQueuesmap[tId]);
            if (Thread::GetLocalQueue()->size() <=
                s_subQueueMaxSize->getValue()) {  // 优先加入到本地队列呢
                Thread::GetLocalQueue()->push_back(fiC);
                //  localcount++;
                //  if(localcount %50000 == 0)
                // LANE_LOG_DEBUG(LANE_LOG_ROOT()) << "in sub" <<
                // fiC.m_fiber->GetFiberId();
            } else {
                m_mainQueue.push(fiC);
                // maincount++;
                // if(maincount%50000 == 0)
                // LANE_LOG_DEBUG(LANE_LOG_ROOT()) << "in main"<<
                // fiC.m_fiber->GetFiberId();
            }
        }

        return needTickle;
    }

private:
    uint32_t    m_threadCount;
    std::string m_name;
    // std::list<FiberAndThread> m_tasks;

    std::vector<Thread::ptr> m_threadPool;

    std::vector<pid_t>    m_threadIds;
    std::atomic<uint32_t> m_activeThreadCount = {0};
    std::atomic<uint32_t> m_idleThreadCount = {0};

    lane::ThreadSafeQueue<FiberAndThread> m_mainQueue;
    // std::vector<lane::WorkStealQueue<FiberAndThread>> m_subQueues;
    // std::vector<std::shared_ptr<lane::WorkStealQueue<FiberAndThread>>>
    //     m_subQueuesptr;

    std::unordered_map<pid_t, int> m_threadIdToQueuIndex;

    Fiber::ptr m_rootFiber;
    pid_t      m_rootThreadId;
    bool       m_stop;
    MutexType  m_mutex;

public:
    std::unordered_map<pid_t, lane::WorkStealQueue<FiberAndThread>*>
        m_subQueuesmap;
};
}  // namespace lane

#endif