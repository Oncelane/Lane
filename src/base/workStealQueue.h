#pragma once

#include <deque>
#include <mutex>
#include <vector>

#include "base/mutex.h"
namespace lane {

template <typename T>
class WorkStealQueue {
public:
    WorkStealQueue() {}


    void push_front(T& item) {
        MutexType::Lock lock(m_mutex);
        m_queue.push_front(item);
    }

    void push_back(T& item) {
        MutexType::Lock lock(m_mutex);
        m_queue.push_back(item);
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
        MutexType::Lock lock(m_mutex);
        if (m_queue.empty()) {
            return false;
        }
        out = std::move(m_queue.front());
        m_queue.pop_front();
        return true;
    }

    std::shared_ptr<T> try_pop() {
        MutexType::Lock lock(m_mutex);
        if (m_queue.empty()) {
            return std::shared_ptr<T>();
        }
        std::shared_ptr<T> out(std::make_shared<T>(std::move(m_queue.front())));
        m_queue.pop_front();
        return out;
    }

    bool try_steal(T& out) {
        MutexType::Lock lock(m_mutex);
        if (m_queue.empty()) {
            return false;
        }
        out = std::move(m_queue.back());
        m_queue.pop_back();
        return true;
    }

    bool try_steal(std::vector<T>& outVec) {
        MutexType::Lock lock(m_mutex);
        if (m_queue.empty()) {
            return false;
        }
        for (int i = 0; i < m_queue.size() / 2; ++i) {
            outVec.push_back(std::move(m_queue.back()));
            m_queue.pop_back();
        }
        return outVec.size();
    }

private:
    using MutexType = QSpinlock;
    std::deque<T>     m_queue;
    mutable MutexType m_mutex;
};

}  // namespace lane