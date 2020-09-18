#pragma once
#include <variant>
#include <system_error>
#include <iostream>
#include <experimental/coroutine>

namespace stde = std::experimental;

template<typename T, typename Error = std::error_code>
class result
{
public:

    using value_type = std::remove_cvref_t<T>;
    using error_type = std::remove_cvref_t<Error>;


    
    //result(const typename std::enable_if_t<std::is_copy_constructible<value_type>::value, value_type>& t)
    //    :mVar(t)
    //{}

    //result(typename std::enable_if_t<std::is_move_constructible<value_type>::value, value_type>&& t)
    //    :mVar(std::forward<value_type>(t))
    //{}

    //result(const typename std::enable_if_t<std::is_copy_constructible<error_type>::value, error_type>& t)
    //    :mVar(t)
    //{}

    //result(typename std::enable_if_t<std::is_move_constructible<error_type>::value, error_type>&& t)
    //    :mVar(std::forward<error_type>(t))
    //{}


    std::variant<value_type, error_type>& var() {
        return coroutine_handle.promise().mVar;
    };
    const std::variant<value_type, error_type>& var() const {
        return coroutine_handle.promise().mVar;
    };


    bool has_value()
    {
        return std::holds_alternative<value_type>(var());
    }
    
    bool has_error()
    {
        return !has_value();
    }

    operator bool()
    {
        return has_error();
    }

    value_type& unwrap()
    {
        if (has_error())
            throw std::runtime_error("unwrap() was called on a Result<T,E> which stores an error_type");

        return std::get<value_type>(var());
    }

    const value_type& unwrap() const
    {
        if (has_error())
            throw std::runtime_error("unwrap() was called on a Result<T,E> which stores an error_type");

        return std::get<const value_type>(var());
    }

    value_type& unwrapOr(value_type& alt)
    {
        if (has_error())
            return alt;
        return std::get<value_type>(var());
    }

    const value_type& unwrapOr(const value_type& alt) const
    {
        if (has_error())
            return alt;
        return std::get<const value_type>(var());
    }

    error_type& error()
    {
        if (has_value())
            throw std::runtime_error("error() was called on a Result<T,E> which stores an value_type");

        return std::get<error_type>(var());
    }

    const error_type& error() const
    {
        if (has_value())
            throw std::runtime_error("error() was called on a Result<T,E> which stores an value_type");

        return std::get<const error_type>(var());
    }


    struct promise_type 
    {
        std::variant<value_type, error_type> mVar;

        result get_return_object() {
            return stde::coroutine_handle<promise_type>::from_promise(*this);
        }
        stde::suspend_never initial_suspend() { return {}; }
        stde::suspend_never final_suspend() noexcept { return {}; }
        void unhandled_exception() {
            auto exceptionPtr = std::current_exception();
            if (exceptionPtr)
                std::rethrow_exception(exceptionPtr);
        }
        void return_value(result value) {mVar = value; };

        struct ResultAwaiter {
            result mRes;

            ResultAwaiter(result&& res) : mRes(std::move(res)) {}

            bool await_ready() { return true; }
            void await_suspend(stde::coroutine_handle<promise_type> coro_handle) {}
            result await_resume() {
                return mRes;
            }
        };

        ResultAwaiter await_transform(result&& s) {
            return ResultAwaiter(std::move(s));
        }


        struct ValueAwaiter {
            value_type mRes;

            ValueAwaiter(value_type&& res) : mRes(std::move(res)) {}

            bool await_ready() { return true; }
            void await_suspend(stde::coroutine_handle<promise_type> coro_handle) {}
            value_type await_resume() {
                return mRes;
            }
        };

        ValueAwaiter await_transform(value_type&& s) {
            return ValueAwaiter(std::move(s));
        }        
        
        struct ErrorAwaiter {
            error_type mRes;

            ErrorAwaiter(error_type&& res) : mRes(std::move(res)) {}

            bool await_ready() { return true; }
            void await_suspend(stde::coroutine_handle<promise_type> coro_handle) {}
            error_type await_resume() {
                return mRes;
            }
        };

        ErrorAwaiter await_transform(error_type&& s) {
            return ErrorAwaiter(std::move(s));
        }


    };

    using coro_handle = std::experimental::coroutine_handle<promise_type>;

    //private:
    coro_handle coroutine_handle;

    result(coro_handle* handle)
        : coroutine_handle(handle)
    {}
};

//
//inline result<int> bar()
//{
//    co_await 3;
//}
//
//inline result<int> foo()
//{
//    int i = co_await bar();
//
//    co_await i;
//}
//
//void ResultTest()
//{
//    std::cout << foo().unwrap() << std::endl;
//}