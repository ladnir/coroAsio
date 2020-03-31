#pragma once
#include <iostream>
#include <vector>
#include "cppcoro/task.hpp"
#include "cppcoro/generator.hpp"
#include "cppcoro/async_generator.hpp"

enum class NetCode
{
    success = 0,
    endOfRound,
    error
};

namespace stde = std::experimental;

class Protocol
{
public:
    struct promise
    {
        NetCode mCode;

        using coro_handle = std::experimental::coroutine_handle<promise>;

        auto get_return_object() {
            return coro_handle::from_promise(*this);
        }
        auto initial_suspend() { return std::experimental::suspend_always(); }
        auto final_suspend() noexcept { return std::experimental::suspend_always(); }
        auto yield_value(const NetCode& str) {
            mCode = str;
            return std::experimental::suspend_always{};
        }

        auto return_void() { 
            if (mCode != NetCode::error) 
                mCode = NetCode::success; 
        }

        void unhandled_exception() {
            std::terminate();
        }
    };


    using promise_type = promise;
    using coro_handle = promise_type::coro_handle;

    bool await_ready() { 
        return !handle_.done(); 
    }
    void await_suspend(stde::coroutine_handle<promise_type> coro_handle) {}

    NetCode await_resume() {
        return resume();
    }



    Protocol(coro_handle handle) : handle_(handle) { assert(handle); }
    Protocol(Protocol&) = delete;
    Protocol(Protocol&&) = delete;

    NetCode resume() {
        if (handle_ == nullptr)
            return NetCode::error;

        if (!handle_.done())
            handle_.resume();

        return handle_.promise().mCode;
    }

    

    ~Protocol() { handle_.destroy(); }
private:
    coro_handle handle_;
};


class Channel {
public:
    template<typename T>
    cppcoro::task<NetCode> send(T)
    {
        co_return NetCode::success; 
    }

    template<typename T>
    cppcoro::task<NetCode> recv(T)
    {
        co_return NetCode::success;
    }
};

inline Protocol foo(Channel&chl)
{
    std::vector<char> buff;
    co_await chl.send(buff);

    co_yield NetCode::endOfRound;

    co_await chl.recv(buff);
    co_await chl.send(buff);

    co_yield NetCode::endOfRound;

    co_await chl.recv(buff);
    co_await chl.send(buff);

}

inline Protocol bar()
{
    Channel chl;
    std::vector<char> data;

    NetCode c = co_await chl.recv(data);
    if (c != NetCode::success)
        co_yield NetCode::error;

    Protocol subprotocol = foo(chl);

    c = co_await subprotocol;

    // give the other protocols a chance to recv/send
    if (c != NetCode::endOfRound)
        co_yield NetCode::error;
    else
        co_yield NetCode::endOfRound;


    co_await chl.recv(data);
    c = co_await subprotocol;

    co_await chl.recv(data);
}


inline void protoMain()
{
    auto proto = bar();
    NetCode code;
    while ((code = proto.resume()) == NetCode::endOfRound);

    std::cout << "cone with code " << (int)code << std::endl;
}