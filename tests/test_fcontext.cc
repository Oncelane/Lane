#include <boost/context/detail/fcontext.hpp>
#include <iostream>

class Fiber {
public:
    using fcontext_t = boost::context::detail::fcontext_t;
    using transfer_t = boost::context::detail::transfer_t;

    Fiber(void (*func)(transfer_t)) {
        // 创建堆栈
        stack = new char[stack_size];
        // 创建上下文
        fctx = boost::context::detail::make_fcontext(
            stack + stack_size, stack_size, func);
    }

    ~Fiber() {
        delete[] stack;  // 释放堆栈内存
    }

    void swap_in() {
        // 从主线程跳转到协程
        // 这里传递当前 Fiber 的指针，以便后续在 fiber_function 中使用
        transfer_t tr = boost::context::detail::jump_fcontext(fctx, this);
        // 在协程中返回时，这里会继续执行
        std::cout << "Returned from fiber." << std::endl;
    }

    void swap_out() {
        // 从协程跳转到主线程
        // 可以传递数据回主线程，设置为为 nullptr 表示无数据
        transfer_t tr = {fctx, this};
        boost::context::detail::jump_fcontext(tr.fctx, tr.data);
    }

private:
    static const std::size_t stack_size = 32768;  // 协程的栈大小
    char*                    stack;               // 存储协程的栈
    fcontext_t               fctx;                // 协程上下文
};

void fiber_function(boost::context::detail::transfer_t tr) {
    std::cout << "Fiber is running." << std::endl;

    // 通过 `tr.data` 获取 Fiber 实例
    Fiber* fiber = static_cast<Fiber*>(tr.data);

    // 在这里可以实现一些协程的逻辑...

    std::cout << "Fiber is about to swap out." << std::endl;

    // 现在可以安全地调用 swap_out
    fiber->swap_out();
}

int main() {
    Fiber fiber(fiber_function);
    fiber.swap_in();  // 从主线程切换到协程执行
    std::cout << "Back to main thread." << std::endl;

    return 0;
}
