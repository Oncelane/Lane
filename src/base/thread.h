/*******************************************
 * Author : Lane
 * Email: : 1657015850@qq.com
 * CreateTime : 2023-01-29 15:35
 * LastModified : 2023-01-29 15:35
 * Filename : thread.h
 * Description : 1.0 *
 ************************************/

#ifndef __LANE_THREAD_H__

#define __LANE_THREAD_H__
#include <pthread.h>

#include <atomic>
#include <functional>
#include <memory>
#include <string>

#include "base/log.h"
#include "base/mutex.h"
#include "base/noncopyable.h"
#include "base/util.h"
#include "base/workStealQueue.h"
namespace lane {
struct FiberAndThread;
class Thread : Noncopyable {
public:
    typedef std::shared_ptr<Thread>   ptr;
    typedef std::unique_ptr<Thread>   unique_ptr;
    typedef std::function<void(void)> CallBackType;

public:
    Thread(const CallBackType& cb, const std::string& name = "");
    ~Thread();

public:
    static const std::string& GetName();

    static pid_t GetTid();

public:
    void join();

    const std::string& getName();
    pid_t              getTid() const {
        return m_id;
    }
    pthread_t getpthreadTid() const {
        return m_pthread;
    }

    static void                                      InitLocalQueue();
    static lane::WorkStealQueue<FiberAndThread>::ptr GetLocalQueue();
    static void                                      DeleteLocalQueue();

private:
    void setDefaultName();

private:
    static void* Main(void* arg);

private:
    // 真实线程（进程）id
    pid_t m_id;
    // Posix定义的id
    pthread_t m_pthread;
    // 线程名
    std::string m_name;
    // 线程回调函数
    CallBackType m_cb;
    Semaphore    m_sem;
    // 线程本地队列
    lane::WorkStealQueue<FiberAndThread>* m_queue;

    static std::atomic<int32_t> ThreadCount;
};
}  // namespace lane

#endif