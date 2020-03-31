#include "examples.h"



#include <iostream>
#include <experimental/coroutine>
#include <cassert>

namespace example1
{


    struct promise
    {
        using coro_handle = std::experimental::coroutine_handle<promise>;
        std::exception_ptr mEPtr = nullptr;

        auto get_return_object() {
            return coro_handle::from_promise(*this);
        }
        auto initial_suspend() { return std::experimental::suspend_always(); }
        auto final_suspend() noexcept { return std::experimental::suspend_always(); }
        void return_void() {}
        void unhandled_exception() {
            mEPtr = std::current_exception();
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

            if (handle_.promise().mEPtr)
                std::rethrow_exception(handle_.promise().mEPtr);

            return !handle_.done();
        }
        ~resumable() { handle_.destroy(); }
    private:


        coro_handle handle_;
    };

    std::exception_ptr myEPtr = nullptr;


    resumable foo() {
        // compiler generates:
        // promise prom = new resumable::promise_type();
        // std::experimental::coroutine_handle<promise> handle =
        //     prom.get_return_object();
        //
        // co_await prom.initial_suspend();
        // if(...suspend...) {
        //    return resumable(handle);   
        //
        //    ... handle.resume() called ...
        // }
        // try {

        try {
            throw std::runtime_error("test");
        }
        catch (...)
        {
            myEPtr = std::current_exception();
            throw;
        }

        std::cout << "Hello" << std::endl;

        co_await std::experimental::suspend_always();
        //  compiler generates:
        //  if(...suspend...) {
        //      if(...first suspend...)
        //          return resumable(handle);
        //      else
        //          return;
        //  
        //     ... handle.resume() called ...
        //  }

        std::cout << "World" << std::endl;

        //  promise.return_void();
        //
        // }
        // catch (...) {
        //    promise.unhandled_exception();
        // }
        // co_await promise.final_suspend();
    }


    int main(int argc, char** argv)
    {
        using namespace example1;

        resumable res = foo();
        try {

            while (res.resume() == false);
        }
        catch (std::runtime_error & rt)
        {
            std::cout << rt.what() << ", eq " << int(myEPtr == std::current_exception()) << std::endl;
        }
        catch (...)
        {
            std::cout << "unhandled exception" << std::endl;            
        }


        return 0;
    }
}

