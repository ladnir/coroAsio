#pragma once
#include <variant>
#include <system_error>
#include <iostream>
#include <experimental/coroutine>

namespace stde = std::experimental;

template<typename T, typename Error = std::error_code>
class Result
{
public:

    using value_type = std::remove_cvref_t<T>;
    using error_type = std::remove_cvref_t<Error>;


    
    //Result(const typename std::enable_if_t<std::is_copy_constructible<value_type>::value, value_type>& t)
    //    :mVar(t)
    //{}

    //Result(typename std::enable_if_t<std::is_move_constructible<value_type>::value, value_type>&& t)
    //    :mVar(std::forward<value_type>(t))
    //{}

    //Result(const typename std::enable_if_t<std::is_copy_constructible<error_type>::value, error_type>& t)
    //    :mVar(t)
    //{}

    //Result(typename std::enable_if_t<std::is_move_constructible<error_type>::value, error_type>&& t)
    //    :mVar(std::forward<error_type>(t))
    //{}


    std::variant<value_type, error_type>& var() {
        return coroutine_handle.promise().mVar;
    };
    const std::variant<value_type, error_type>& var() const {
        return coroutine_handle.promise().mVar;
    };


    bool hasValue()
    {
        return std::holds_alternative<value_type>(var());
    }
    
    bool hasError()
    {
        return !hasValue();
    }

    operator bool()
    {
        return hasError();
    }

    value_type& unwrap()
    {
        if (hasError())
            throw std::runtime_error("unwrap() was called on a Result<T,E> which stores an error_type");

        return std::get<value_type>(var());
    }

    const value_type& unwrap() const
    {
        if (hasError())
            throw std::runtime_error("unwrap() was called on a Result<T,E> which stores an error_type");

        return std::get<const value_type>(var());
    }

    value_type& unwrapOr(value_type& alt)
    {
        if (hasError())
            return alt;
        return std::get<value_type>(var());
    }

    const value_type& unwrapOr(const value_type& alt) const
    {
        if (hasError())
            return alt;
        return std::get<const value_type>(var());
    }

    error_type& error()
    {
        if (hasValue())
            throw std::runtime_error("error() was called on a Result<T,E> which stores an value_type");

        return std::get<error_type>(var());
    }

    const error_type& error() const
    {
        if (hasValue())
            throw std::runtime_error("error() was called on a Result<T,E> which stores an value_type");

        return std::get<const error_type>(var());
    }


    struct promise_type 
    {
        std::variant<value_type, error_type> mVar;

        Result get_return_object() {
            return stde::coroutine_handle<promise_type>::from_promise(*this);
        }
        stde::suspend_never initial_suspend() { return {}; }
        stde::suspend_never final_suspend() noexcept { return {}; }
        void unhandled_exception() {
            auto exceptionPtr = std::current_exception();
            if (exceptionPtr)
                std::rethrow_exception(exceptionPtr);
        }
        void return_value(Result value) {mVar = value; };

        struct ResultAwaiter {
            Result mRes;

            ResultAwaiter(Result&& res) : mRes(std::move(res)) {}

            bool await_ready() { return true; }
            void await_suspend(stde::coroutine_handle<promise_type> coro_handle) {}
            Result await_resume() {
                return mRes;
            }
        };

        ResultAwaiter await_transform(Result&& s) {
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

    Result(coro_handle* handle)
        : coroutine_handle(handle)
    {}
};

//
//inline Result<int> bar()
//{
//    co_await 3;
//}
//
//inline Result<int> foo()
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