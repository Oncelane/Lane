
#include <list>
#include "base/noncopyable.h"
#include "base/mutex.h"

namespace lane
{

    template <class T>
    class channel : Noncopyable
    {
    public:
        channel(uint32_t capacity) : m_space(capacity), m_item(0){};
        ~channel() {}

        T  output() {
            m_item.wait();
            T tmp = m_list.front();
            m_list.pop_front();
            m_space.post();
            return tmp;
        }
        void input(T tmp) {
            m_space.wait();
            m_list.push_back(tmp);
            m_item.post();
            return;
        }
    private:
        FiberSemaphore m_space;
        FiberSemaphore m_item;
        std::list<T> m_list;
    };

}; // namespace lane