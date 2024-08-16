#include <algorithm>
#include <boost/context/detail/fcontext.hpp>
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>

#include "base/fiber.h"

using namespace boost::context::detail;
class fFiber;
static thread_local fFiber* t_curFiber;
static thread_local fFiber* t_threadFiber;
class fFiber : public std::enable_shared_from_this<fFiber> {
private:
public:
    using ptr = fFiber*;
    using callBack = std::function<void(void)>;

    fFiber() {
        //空的，只是用来放main fiber
    }

    fFiber(callBack _cb) : m_cb(_cb), m_stackSize(1024 * 4) {
        //分配内存
        m_stack = (char*)malloc(m_stackSize);
        m_fctx = boost::context::detail::make_fcontext(
            m_stack + m_stackSize, m_stackSize, MainFunc);
    }

    void swapIn() {
        t_curFiber = this;
        assert(fFiber::GetThis() != t_curFiber);
        boost::context::detail::jump_fcontext(m_fctx, this);
    }

    void swapOut() {
        jump_fcontext(t_threadFiber->m_fctx, nullptr);
    }

    static void MainFunc(transfer_t in) {
        t_threadFiber->m_fctx = in.fctx;
        fFiber* readyFiber = reinterpret_cast<fFiber*>(in.data);
        readyFiber->m_cb();
        readyFiber->swapOut();
    }

    static fFiber::ptr GetThis() {
        if (t_threadFiber == nullptr) {
            t_threadFiber = new fFiber();
        }
        return t_threadFiber;
    }

    static void SetThis(fFiber::ptr in) {
        t_curFiber = in;
    }

    void*            private_data;
    fcontext_t       m_fctx;
    fFiber::callBack m_cb;
    char*            m_stack;
    size_t           m_stackSize;
};

void testFiber() {
    auto f1 = fFiber([]() {
        printf("enter f1\n");
        printf("end f1\n");
    });

    auto f2 = fFiber([]() {
        printf("enter f2\n");
        printf("end f2\n");
    });

    printf("main -> f1\n");
    f1.swapIn();
    printf("f1 -> main\n");

    printf("main -> f2\n");
    f2.swapIn();
    printf("f2 -> main\n");

    printf("main exit\n");
}

uint count = 0;

void benchmark_fcontext() {

    for (int i = 0; i < 1000000; ++i) {
        fFiber([]() { count++; }).swapIn();
    }
}

void benchmark_ucontext() {
    for (int i = 0; i < 10; ++i) {
        lane::Fiber(
            []() {
                printf("%d \n", count);
                count++;
            },
            true,
            1024 * 4)
            .swapIn();
    }
}

int main() {
    // benchmark_fcontext();
    benchmark_ucontext();
}