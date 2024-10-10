#pragma once

#include <cstdint>
#include <deque>
#include <memory>
#include <utility>
#include <vector>

#include "base/mutex.h"
namespace lane {

template <typename T>
class WorkStealQueue {
public:
    using ptr = std::shared_ptr<WorkStealQueue<T>>;

    WorkStealQueue() = delete;
    WorkStealQueue(uint32_t cap) : m_cap(cap) {}

    bool push_front(T item) {
        MutexType::Lock lock(m_mutex);
        if (m_cap == m_queue.size()) {
            return false;
        }
        m_queue.push_front(item);
        return true;
    }

    bool push_back(T item) {
        // MutexType::Lock lock(m_mutex);
        m_mutex.lock();
        if (m_cap == m_queue.size()) {
            m_mutex.unlock();
            return false;
        }
        m_queue.push_back(std::move(item));
        m_mutex.unlock();
        return true;
    }

    bool push_back_if_empty(T item) {
        MutexType::Lock lock(m_mutex);
        if (m_queue.size() == 0) {
            m_queue.push_back(item);
            return true;
        }
        return false;
    }

    T pop_front() {
        MutexType::Lock lock(m_mutex);
        auto            out = m_queue.front();
        m_queue.pop_front();
        return out;
    }

    T pop_back() {
        MutexType::Lock lock(m_mutex);
        auto            out = m_queue.back();
        m_queue.pop_back();
        return out;
    }

    T front() {
        MutexType::Lock lock(m_mutex);
        return m_queue.front();
    }

    T back() {
        MutexType::Lock lock(m_mutex);
        return m_queue.back();
    }

    bool empty() const {
        MutexType::Lock lock(m_mutex);
        return m_queue.empty();
    }

    size_t size() const {
        return m_queue.size();
    }

    void clear() {
        MutexType::Lock lock(m_mutex);
        auto            tmp = std::deque<T>();
        m_queue.swap(tmp);
    }

    bool try_pop(T& out) {
        // MutexType::Lock lock(m_mutex);
        m_mutex.lock();
        if (m_queue.empty()) {
            m_mutex.unlock();
            return false;
        }
        out = std::move(m_queue.front());
        m_queue.pop_front();
        m_mutex.unlock();
        return true;
    }

    std::shared_ptr<T> try_pop() {
        // MutexType::Lock lock(m_mutex);
        m_mutex.lock();
        if (m_queue.empty()) {
            m_mutex.unlock();
            return nullptr;
        }
        std::shared_ptr<T> out(std::make_shared<T>(std::move(m_queue.front())));
        m_queue.pop_front();
        m_mutex.unlock();
        return out;
    }

    bool try_pop(std::vector<T>& outVec) {
        MutexType::Lock lock(m_mutex);
        for (int i = 0; i < m_queue.size(); ++i) {
            outVec.push_back(std::move(m_queue.back()));
            m_queue.pop_back();
        }
        return outVec.size();
    }

    bool try_steal(T& out) {
        // MutexType::Lock lock(m_mutex);
        m_mutex.lock();
        if (m_queue.empty()) {
            m_mutex.unlock();
            return false;
        }
        out = std::move(m_queue.back());
        m_queue.pop_back();
        m_mutex.unlock();
        return true;
    }

    bool try_steal(std::vector<T>& outVec) {
        MutexType::Lock lock(m_mutex);
        if (m_queue.empty()) {
            return false;
        }
        for (long unsigned int i = 0; i < (m_queue.size() + 1) / 2; ++i) {
            outVec.push_back(m_queue.back());
            m_queue.pop_back();
        }
        return outVec.size();
    }

private:
    using MutexType = lane::SpinMutex;
    std::deque<T>     m_queue;
    mutable MutexType m_mutex;
    uint32_t          m_cap;
};

}  // namespace lane