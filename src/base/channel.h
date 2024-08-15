
#include <list>

#include "base/mutex.h"
#include "base/noncopyable.h"

namespace lane {

template <class T>
class channel : Noncopyable {
public:
    channel(uint32_t capacity)
        : m_space(capacity), m_item(0), m_close(false), m_size(0) {};
    ~channel() {}

    T output() {
        m_item.wait();
        T tmp = m_list.front();
        m_list.pop_front();
        m_space.post();
        --m_size;
        return tmp;
    }
    void input(T tmp) {
        m_space.wait();
        m_list.push_back(tmp);
        m_item.post();
        ++m_size;
        return;
    }
    // no lock
    uint32_t size() {
        return m_size;
    }

private:
    FiberSemaphore m_space;
    FiberSemaphore m_item;
    std::list<T>   m_list;
    bool           m_close;
    uint32_t       m_size;
};

};  // namespace lane