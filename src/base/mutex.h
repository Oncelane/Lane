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
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>

#include <memory>
#include <queue>

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
    Mutex();
    ~Mutex();
    void lock();
    void unlock();

private:
    pthread_mutex_t m_mutex;
};

class SpinMutex : Noncopyable {
public:
    typedef ScopedLockImpl<SpinMutex> Lock;

public:
    SpinMutex();
    ~SpinMutex();
    void lock();
    void unlock();

private:
    pthread_spinlock_t m_mutex;
};

class RWMutex : Noncopyable {
public:
    typedef ReadScopedLockImpl<RWMutex>  ReadLock;
    typedef WriteScopedLockImpl<RWMutex> WriteLock;

public:
    RWMutex();
    ~RWMutex();
    void rdLock();

    void wrLock();

    void unlock();

private:
    pthread_rwlock_t m_rwMutex;
};

class FiberSemaphore : Noncopyable {
public:
    FiberSemaphore(uint32_t count = 0);
    ~FiberSemaphore();
    void    wait();
    void    wait(Mutex& mu);
    bool    waitForSeconds(time_t seconds);
    void    post();
    void    post(Mutex& mu);
    int32_t getSem();
    void    reset();
    void    resize(int32_t size);

private:
    std::queue<std::pair<IOManager*, std::shared_ptr<Fiber>>> m_waitQueue;
    int32_t                                                   m_sem;
    Mutex                                                     m_mutex;  // 因为需要对std::list进行增删查改，所以必须用互斥锁
};

class FiberMutex : Noncopyable {
public:
    typedef ScopedLockImpl<FiberMutex> Lock;

    FiberMutex() : m_fs(1) {}
    ~FiberMutex();
    void lock();
    void unlock();

private:
    FiberSemaphore m_fs;
};

}  // namespace lane

#endif