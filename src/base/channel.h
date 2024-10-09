
#include <sys/types.h>

#include <cstdint>
#include <functional>
#include <optional>
#include <queue>

#include "base/iomanager.h"
#include "base/log.h"
#include "base/macro.h"
#include "base/mutex.h"
#include "base/noncopyable.h"

namespace lane {

template <class T>
class Channel : Noncopyable {
public:
    enum Type {
        conflated,
        unbuffered,
        buffered,
    };
    static const uint32_t CONFLATED = -1;
    Channel() = delete;
    Channel(uint32_t capacity)
        : m_space(capacity)
        , m_item(0)
        , m_close(false)
        , m_cap(capacity)
        , m_size(0) {
        if (m_cap == CONFLATED) {
            m_type = conflated;
            // cold channel
        } else if (m_cap == 0) {
            m_type = unbuffered;

            // unbuffer channel
        } else if (m_cap < 0) {
            LANE_ASSERT2(false, "init Channel cap < 0");
            // exception
        } else {
            m_type = buffered;
            // normal buffer channel
        }
    };
    ~Channel() {}

    std::optional<T> output() {
        Mutex::Lock lock(m_mu);
        if (m_close) {
            return std::nullopt;
        }
        switch (m_type) {
            case unbuffered: {

                m_item.wait(m_mu);
                if (m_close && m_queue.empty()) {
                    return std::nullopt;
                }
                T out = m_queue.front();
                m_queue.pop();
                --m_size;

                m_space.post(m_mu);
                return out;
                break;
            }
            case buffered: {
                m_item.wait(m_mu);

                if (m_close && m_queue.empty()) {
                    return std::nullopt;
                }
                T out = m_queue.front();
                m_queue.pop();
                --m_size;

                m_space.post(m_mu);
                return out;
                break;
            }
            case conflated: {

                while (m_queue.empty()) {

                    auto iom = IOManager::GetThis();
                    iom->addBlock();
                    m_waitQueue.emplace_back(iom, Fiber::GetThis());

                    Fiber::YieldToHold();
                }
                T out = m_queue.front();
                m_queue.pop();
                --m_size;

                return out;
                break;
            }
        }
        return std::nullopt;
    }

    bool input(T in) {
        Mutex::Lock lock(m_mu);
        switch (m_type) {
            case conflated: {

                if (!m_queue.empty()) {
                    m_queue.pop();
                }
                m_queue.push(in);
                ++m_size;
                if (!m_waitQueue.empty()) {
                    auto wake = m_waitQueue.front();
                    wake.first->schedule(wake.second);
                    wake.first->delBlock();
                }

                break;
            }
            case unbuffered: {

                if (m_close) {

                    return false;
                }
                if (m_queue.empty()) {
                    m_queue.push(in);
                    ++m_size;
                    m_item.post(m_mu);

                    m_space.wait(m_mu);
                } else {
                    m_space.wait(m_mu);

                    if (m_close) {

                        return false;
                    }
                    m_queue.push(in);
                    ++m_size;

                    m_item.post(m_mu);
                }

                break;
            }
            case buffered: {
                m_space.wait(m_mu);
                if (m_close) {

                    return false;
                }
                m_queue.push(in);
                ++m_size;
                m_item.post(m_mu);
                break;
            }
        }

        return true;
    }
    void range(std::function<void(T)> cb) {
        while (true) {
            auto out = output();
            if (out.has_value()) {
                cb(out.value());
            } else {
                break;
            }
        }
    }
    bool close() {
        Mutex::Lock lock(m_mu);
        // if (m_close) {
        //     return false;
        // }
        m_close = true;
        m_space.resize(INT8_MAX);
        m_item.resize(INT8_MAX);
        m_space.resize(INT8_MAX);
        m_item.resize(INT8_MAX);
        return true;
    }


    void reset(uint32_t capacity) noexcept {
        m_space.resize(capacity);
        m_item.resize(0);
        m_close = false;
        m_cap = capacity;
        m_size = 0;
        std::queue<T> newQueue;
        m_queue.swap(newQueue);

        if (m_cap == CONFLATED) {
            m_type = conflated;
            // cold channel
        } else if (m_cap == 0) {
            m_type = unbuffered;

            // unbuffer channel
        } else if (m_cap < 0) {
            LANE_ASSERT2(false, "init Channel cap < 0");
            // exception
        } else {
            m_type = buffered;
            // normal buffer channel
        }
    }
    // no lock
    uint32_t size() noexcept {
        return m_size;
    }

private:
    FiberSemaphore                                           m_space;
    FiberSemaphore                                           m_item;
    Mutex                                                    m_mu;
    std::queue<T>                                            m_queue;
    bool                                                     m_close;
    uint32_t                                                 m_cap;
    uint32_t                                                 m_size;
    u_int8_t                                                 m_type;
    std::list<std::pair<IOManager*, std::shared_ptr<Fiber>>> m_waitQueue;
};

};  // namespace lane