#include "examples.h"



#include <iostream>
#include <experimental/coroutine>
#include <cassert>

namespace example1
{


    struct promise
    {
        using coro_handle = std::experimental::coroutine_handle<promise>;

        auto get_return_object() {
            return coro_handle::from_promise(*this);
        }
        auto initial_suspend() { return std::experimental::suspend_always(); }
        auto final_suspend() noexcept { return std::experimental::suspend_always(); }
        void return_void() {}
    };

    class resumable {
    public:
        using promise_type = promise;
        using coro_handle = promise_type::coro_handle;

        resumable(coro_handle handle) : handle_(handle) { assert(handle); }
        void resume() { handle_.resume(); }
        ~resumable() { handle_.destroy(); }

        coro_handle handle_;
    };

    resumable foo() {

        throw std::runtime_error("test");
        while (true)
            co_await std::experimental::suspend_always();
    }

    int main(int argc, char** argv)
    {

        {
            resumable res = foo();
            try {
                while (true)
                    res.resume();
            }
            catch (std::runtime_error & rt)
            {
                std::cout << rt.what() << std::endl;
            }
        }

        {
            try {

                resumable res = foo();
                while (true)
                    res.resume();
            }
            catch (std::runtime_error & rt)
            {
                std::cout << rt.what() << std::endl;
            }
        }


        return 0;
    }
}

