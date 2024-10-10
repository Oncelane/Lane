#pragma once

#include <deque>
#include <memory>
#include <mutex>
#include <vector>

namespace lane {

template <typename T>
class WorkStealQueue {
public:
    using ptr = std::shared_ptr<WorkStealQueue<T>>;
    WorkStealQueue() {}

    void push_front(T item) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push_front(item);
    }

    void push_back(T item) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push_back(item);
    }

    bool push_back_if_empty(T item) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_queue.size() == 0) {
            m_queue.push_back(item);
            return true;
        }
        return false;
    }

    T pop_front() {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto out = m_queue.front();
        m_queue.pop_front();
        return out;
    }

    T pop_back() {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto out = m_queue.back();
        m_queue.pop_back();
        return out;
    }

    T front() {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.front();
    }

    T back() {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.back();
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }

    size_t size() const {
        return m_queue.size();
    }

    void clear() {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto                        tmp = std::deque<T>();
        m_queue.swap(tmp);
    }

    bool try_pop(T& out) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_queue.empty()) {
            return false;
        }
        out = std::move(m_queue.front());
        m_queue.pop_front();
        return true;
    }

    std::shared_ptr<T> try_pop() {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_queue.empty()) {
            return nullptr;
        }
        std::shared_ptr<T> out(std::make_shared<T>(std::move(m_queue.front())));
        m_queue.pop_front();
        return out;
    }

    bool try_pop(std::vector<T>& outVec) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_queue.empty()) {
            return false;
        }
        for (long unsigned int i = 0; i < m_queue.size(); ++i) {
            outVec.push_back(m_queue.back());
            m_queue.pop_back();
        }
        return outVec.size();
    }

    bool try_steal(T& out) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_queue.empty()) {
            return false;
        }
        out = std::move(m_queue.back());
        m_queue.pop_back();
        return true;
    }

    bool try_steal(std::vector<T>& outVec) {
        std::lock_guard<std::mutex> lock(m_mutex);
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
    std::deque<T>      m_queue;
    mutable std::mutex m_mutex;
};

}  // namespace lane