#include "examples.h"



#include <iostream>
#include <experimental/coroutine>
#include <cassert>

namespace example3
{


    struct promise
    {
        std::string mRet;

        using coro_handle = std::experimental::coroutine_handle<promise>;

        auto get_return_object() {
            return coro_handle::from_promise(*this);
        }
        auto initial_suspend() { return std::experimental::suspend_always(); }
        auto final_suspend() noexcept { return std::experimental::suspend_always(); }
        auto yield_value(const std::string& str) {
            mRet = str;
            return std::experimental::suspend_always{};
        }

        auto return_void() {}
        void unhandled_exception() {
            std::terminate();
        }
    };



    class resumable {
    public:

        using promise_type = promise;
        using coro_handle = promise_type::coro_handle;

        resumable(coro_handle handle) : handle_(handle) { assert(handle); }
        resumable(resumable&) = delete;
        resumable(resumable&&) = delete;

        bool resume() {
            if (!handle_.done())
                handle_.resume();
            return !handle_.done();
        }

        std::string recent_val()
        {
            return handle_.promise().mRet;
        }

        ~resumable() { handle_.destroy(); }
    private:
        coro_handle handle_;
    };


    resumable foo() {
        // compiler generates:
        // promise prom = new resumable::promise_type();
        // std::experimental::coroutine_handle<promise> handle =
        //     prom.get_return_object();
        //
        // co_await prom.initial_suspend();
        // ... no suspend since it returns never_suspend type.

        // try {

        while (true) {

            co_yield "Hello";
            // compiler generates:
            // co_await promise.yeild_value("Hello");
            // if(...suspend...) {
            //     if(...first suspend...)
            //         return resumable(handle);
            //     else
            //         return;
            //  
            //     ... handle.result() called ...
            // }

            co_yield "Coroutine";
            // compiler generates:
            // co_await promise.yeild_value("Coroutine");
            // if(...suspend...) {
            //    return;
            //     ... handle.result() called ...
            // }
        }

        // }
        // catch (...) {
        //    promise.unhandled_exception();
        // }
        // co_await promise.final_suspend();
    }


    int main(int argc, char** argv)
    {
        using namespace example3;
        resumable res = foo();
        int i = 10;
        while (i--) {
            res.resume();
            std::cout << res.recent_val() << std::endl;

        }
        return 0;
    }

}