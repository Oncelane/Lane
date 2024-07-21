#pragma once

#include <condition_variable>
#include <queue>
#include <thread>

namespace lane {

    template <typename T>
    class ThreadSafeQueue {
       public:
        ThreadSafeQueue() {}

        void push(T& item) {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_queue.push(item);
            m_cond.notify_one();
        }

        void wait_and_pop(T& value) {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cond.wait(lock, [this] {
                return !m_queue.empty();
            });
            value = m_queue.front();
            m_queue.pop();
        }

        std::shared_ptr<T> wait_and_pop() {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cond.wait(lock, [this] {
                return !m_queue.empty();
            });
            std::shared_ptr<T> ret(
                std::make_shared<T>(std::move(m_queue.front())));
            m_queue.pop();
            return ret;
        }

        bool try_pop(T& value) {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_queue.empty()) {
                return false;
            }
            value = std::move(m_queue.front());
            m_queue.pop();
            return true;
        }

        std::shared_ptr<T> try_pop() {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_queue.empty()) {
                return std::shared_ptr<T>();
            }
            std::shared_ptr<T> ret(
                std::make_shared<T>(std::move(m_queue.front())));
            m_queue.pop();
            return ret;
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
            auto tmp = std::queue<T>();
            m_queue.swap(tmp);
        }

       private:
        mutable std::mutex m_mutex;
        std::queue<T> m_queue;
        std::condition_variable m_cond;
    };

}  // namespace lane