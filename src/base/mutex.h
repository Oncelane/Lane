/*******************************************
 * Author : Lane
 * Email: : 1657015850@qq.com
 * CreateTime : 2023-01-30 16:46
 * LastModified : 2023-01-30 16:46
 * Filename : mutex.h
 * Description : 1.0 *
 ************************************/

#ifndef __LANE_MUTEX_H__
#define __LANE_MUTEX_H__
#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>

#include <cassert>
#include <list>
#include <memory>

#include "base/noncopyable.h"
// #include "base/macro.h"
namespace lane {
class IOManager;
class Fiber;
class Semaphore : Noncopyable {
public:
    Semaphore(uint32_t count = 0);
    ~Semaphore();
    void wait();
    bool waitForSeconds(time_t seconds);
    void post();

private:
    sem_t m_semaphore;
};

template <typename T>
class ScopedLockImpl {
public:
    ScopedLockImpl(T& mutex) : m_mutex(mutex) {
        m_locked = false;
        lock();
    }
    ~ScopedLockImpl() {
        unlock();
    }
    void lock() {
        if (!m_locked) {
            m_locked = true;
            m_mutex.lock();
        }
    }
    void unlock() {
        if (m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }

private:
    T&   m_mutex;
    bool m_locked;
};

template <typename T>
class ReadScopedLockImpl {
public:
    ReadScopedLockImpl(T& rwMutex) : m_rwMutex(rwMutex) {
        m_locked = false;
        lock();
    }
    ~ReadScopedLockImpl() {
        unlock();
    }
    void lock() {
        if (!m_locked) {
            m_locked = true;
            m_rwMutex.rdLock();
        }
    }
    void unlock() {
        if (m_locked) {
            m_rwMutex.unlock();
            m_locked = false;
        }
    }

private:
    T&   m_rwMutex;
    bool m_locked;
};

template <typename T>
class WriteScopedLockImpl {
public:
    WriteScopedLockImpl(T& rwMutex) : m_rwMutex(rwMutex) {
        m_locked = false;
        lock();
    }
    ~WriteScopedLockImpl() {
        unlock();
    }
    void lock() {
        if (!m_locked) {
            m_locked = true;
            m_rwMutex.wrLock();
        }
    }
    void unlock() {
        if (m_locked) {
            m_rwMutex.unlock();
            m_locked = false;
        }
    }

private:
    T&   m_rwMutex;
    bool m_locked;
};

class Mutex : Noncopyable {
public:
    typedef ScopedLockImpl<Mutex> Lock;

public:
    Mutex() {
        assert(pthread_mutex_init(&m_mutex, nullptr) == 0);
    }
    ~Mutex() {
        assert(pthread_mutex_destroy(&m_mutex) == 0);
    }
    void lock() {
        assert(pthread_mutex_lock(&m_mutex) == 0);
    }
    void unlock() {
        assert(pthread_mutex_unlock(&m_mutex) == 0);
    }

private:
    pthread_mutex_t m_mutex;
};

class RWMutex : Noncopyable {
public:
    typedef ReadScopedLockImpl<RWMutex>  ReadLock;
    typedef WriteScopedLockImpl<RWMutex> WriteLock;

public:
    RWMutex() {
        assert(pthread_rwlock_init(&m_rwMutex, nullptr) == 0);
    }
    ~RWMutex() {
        assert(pthread_rwlock_destroy(&m_rwMutex) == 0);
    }
    void rdLock() {
        assert(pthread_rwlock_rdlock(&m_rwMutex) == 0);
    }

    void wrLock() {
        assert(pthread_rwlock_wrlock(&m_rwMutex) == 0);
    }

    void unlock() {
        assert(pthread_rwlock_unlock(&m_rwMutex) == 0);
    }

private:
    pthread_rwlock_t m_rwMutex;
};

class FiberSemaphore : Noncopyable {
public:
    FiberSemaphore(uint32_t count = 0);
    ~FiberSemaphore();
    void   wait();
    void   wait(Mutex& mu);
    bool   waitForSeconds(time_t seconds);
    void   post();
    void   post(Mutex& mu);
    int8_t getSem();
    void   reset();
    void   resize(int8_t size);

private:
    std::list<std::pair<IOManager*, std::shared_ptr<Fiber>>> m_waitQueue;
    int8_t                                                   m_sem;
    Mutex                                                    m_mutex;  // 因为需要对std::list进行增删查改，所以必须用互斥锁
};

class FiberMutex : Noncopyable {
public:
    typedef ScopedLockImpl<FiberMutex> Lock;

    FiberMutex() : m_fs(1) {}
    ~FiberMutex();
    void lock() {
        m_fs.wait();
    }
    void unlock() {
        assert(m_fs.getSem() <= 0);
        m_fs.post();
    }

private:
    FiberSemaphore m_fs;
};


// class FiberCondition : Noncopyable {
// public:
//     FiberCondition();
//     ~FiberCondition();
//     void wait(ScopedLockImpl<FiberMutex>& Lock, std::function<bool(void)>
//     cb); bool notify(); bool notify_all();

// private:
//     std::queue<std::pair<IOManager*, std::shared_ptr<Fiber>>> m_waitQueue;
//     int8_t                                                    m_sem;
//     std::function<bool(void)>                                 m_cb;
// };

}  // namespace lane

#endif