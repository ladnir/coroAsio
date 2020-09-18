#include "examples.h"



#include <iostream>
#include <fstream>
#include <experimental/coroutine>
#include <optional>
#include <cassert>

namespace example4
{

    enum button_press
    {
        LEFT_MOUSE = 0,
        RIGHT_MOUSE
    };

    namespace stde = std::experimental;

    template <typename Signal, typename result> class StateMachine {
    public:


        struct signal {};

        struct promise_type {
            std::optional<Signal> recent_signal;
            std::optional<result> returned_value;
            StateMachine get_return_object() {
                return stde::coroutine_handle<promise_type>::from_promise(*this);
            }
            stde::suspend_never initial_suspend() { return {}; }
            stde::suspend_always final_suspend() noexcept { return {}; }
            void unhandled_exception() {
                auto exceptionPtr = std::current_exception();
                if (exceptionPtr)
                    std::rethrow_exception(exceptionPtr);
            }
            void return_value(result value) { returned_value.emplace(value); };
            
            struct SignalAwaiter {
                std::optional<Signal>& recent_signal;
                SignalAwaiter(std::optional<Signal>& signal) : recent_signal(signal) {}
                bool await_ready() { return recent_signal.has_value(); }
                void await_suspend(stde::coroutine_handle<promise_type> coro_handle) {}
                Signal await_resume() {
                    assert(recent_signal.has_value());

                    Signal tmp = *recent_signal;
                    recent_signal.reset();
                    return *recent_signal;
                }
            };

            SignalAwaiter await_transform(signal s) {
                return SignalAwaiter(recent_signal);
            }
        };

        using coro_handle = std::experimental::coroutine_handle<promise_type>;

        StateMachine(coro_handle coro_handle) : coroutine_handle(coro_handle) {}
        StateMachine(StateMachine&&) = default;
        StateMachine(const StateMachine&) = delete;
        
        ~StateMachine() { coroutine_handle.destroy(); }
        
        void send_signal(Signal signal) {
            coroutine_handle.promise().recent_signal = signal;
            if (!coroutine_handle.done())
                coroutine_handle.resume();
        }
        
        std::optional<result> get_result() {
            return coroutine_handle.promise().returned_value;
        }


    private:
        coro_handle coroutine_handle;
    };

    StateMachine<button_press, int> open_file(const char* file_name)
    {
        using this_coroutine = StateMachine<button_press, int>;

        button_press first_button = co_await this_coroutine::signal{};
        while (true) {
            button_press second_button = co_await this_coroutine::signal{};
            if (first_button == button_press::LEFT_MOUSE and
                second_button == button_press::LEFT_MOUSE)
                co_return 3;
            first_button = second_button;
        }
    }


    int main(int argc, char** argv)
    {
        auto machine = open_file("test");
        machine.send_signal(button_press::LEFT_MOUSE);
        machine.send_signal(button_press::RIGHT_MOUSE);
        machine.send_signal(button_press::LEFT_MOUSE);
        machine.send_signal(button_press::LEFT_MOUSE);
        auto result = machine.get_result();
        std::cout << result.value() << std::endl;

        //if (result.has_value())
        //    std::fclose(*result);

        return 0;
    }

    //using coro_handle = std::experimental::coroutine_handle<promise>;

    //struct A
    //{
    //    bool await_ready();
    //    coro_handle await_suspend(const coro_handle& );
    //    void await_resume();
    //};
    //A a;
    //void suspend_coroutine();
    //void return_to_the_caller();

    //void bar()
    //{
    //    coro_handle coroutine_handle;
    //    std::exception_ptr exception = nullptr;
    //    if (! a.await_ready()) {
    //        suspend_coroutine();
    //        //if await_suspend returns void
    //        try {
    //            a.await_suspend(coroutine_handle);
    //            return_to_the_caller();
    //        }
    //        catch (...) {
    //            exception = std::current_exception();
    //            goto resume_point;
    //        }
    //        //endif
    //        //if await_suspend returns bool
    //        bool await_suspend_result;
    //        try {
    //            await_suspend_result = a.await_suspend(coroutine_handle);
    //        }
    //        catch (...) {
    //            exception = std::current_exception();
    //            goto resume_point;
    //        }
    //        if (! await_suspend_result)
    //            goto resume_point;
    //        return_to_the_caller();
    //        //endif


    //    resume_point:
    //        if (exception)
    //            std::rethrow_exception(exception);
    //        return a.await_resume();


    //        //if await_suspend returns another coroutine_handle
    //        using coro_handle2 = decltype(a.await_suspend(std::declval<coro_handle>()));
    //        coro_handle2 another_coro_handle;
    //        try {
    //            another_coro_handle = a.await_suspend(coroutine_handle);
    //        }
    //        catch (...) {
    //            exception = std::current_exception();
    //            goto resume_point;
    //        }
    //        another_coro_handle.resume();
    //        return_to_the_caller();
    //        //endif
    //    }
    //}
}