#pragma once

#include <experimental/coroutine>
#include <variant>
#include <optional>
#include <system_error>
#include <tuple>
#include <iostream>
#include <string>
#include "cppcoro/task.hpp"

#define INLINE_VARIANT 
#define VERBOSE(x)
//#define VERBOSE(x) x

namespace ncoro
{
    using u64 = unsigned long long;
    using u32 = unsigned int;
    using u8 = unsigned char;
    using i64 = long long;
    using i32 = int;
    using i8 = char;


    enum class Errc
    {
        success = 0,
        uncaught_exception
        //CloseChannel = 1 // error indicating we should call the close handler.
    };
}
namespace std {
    template <>
    struct is_error_code_enum<ncoro::Errc> : true_type {};
}

namespace ncoro
{
    using error_code = std::error_code;

    error_code make_error_code(Errc e);


    class BadResultAccess
        : public std::exception
    {
    public:
        BadResultAccess(char const* const msg)
            : std::exception(msg)
        {
        }
    };

    namespace stde = std::experimental;

    namespace details
    {


        template<typename T, typename Error, typename ExceptionHandler>
        class Result
        {
        public:
            using value_type = std::remove_cvref_t<T>;
            using error_type = std::remove_cvref_t<Error>;
            using exception_handler = std::remove_cvref_t<ExceptionHandler>;


#ifdef INLINE_VARIANT
            Result(const typename std::enable_if_t<std::is_copy_constructible<value_type>::value, value_type>& t)
                :mVar(t)
            {
                VERBOSE(std::cout << "constructed Result 1 at " << (u64)this << " " << *this << std::endl);
            }

            Result(typename std::enable_if_t<std::is_move_constructible<value_type>::value, value_type>&& t)
                :mVar(std::forward<value_type>(t))
            {
                VERBOSE(std::cout << "constructed Result 2 at " << (u64)this << " " << *this << std::endl);
            }

            Result(const typename std::enable_if_t<std::is_copy_constructible<error_type>::value, error_type>& e)
                :mVar(e)
            {
                VERBOSE(std::cout << "constructed Result 3 at " << (u64)this << " " << *this << std::endl);
            }

            Result(typename std::enable_if_t<std::is_move_constructible<error_type>::value, error_type>&& e)
                :mVar(std::forward<error_type>(e))
            {
                VERBOSE(std::cout << "constructed Result 4 at " << (u64)this << " " << *this << std::endl);
            }

            Result(Result&& r)
            {
                var() = r.mVar;
                VERBOSE(std::cout << "constructed Result 5 at " << (u64)this << " " << *this << std::endl);
            }

            std::variant<value_type, error_type>& var() {
                return mVar;
            };
            const std::variant<value_type, error_type>& var() const {
                return mVar;
            };


#else
            ~Result()
            {
                VERBOSE(std::cout << "~Result() " << (u64)this << std::endl);
                if (coroutine_handle)
                    coroutine_handle.destroy();
            }

            Result(const typename std::enable_if_t<std::is_copy_constructible<value_type>::value, value_type>& t)
            {
                *this = [&]()-> Result {co_return t; }();
                VERBOSE(std::cout << "constructed Result 1 at " << (u64)this << " " << *this << std::endl);
            }

            Result(typename std::enable_if_t<std::is_move_constructible<value_type>::value, value_type>&& t)
            {
                *this = [&]()-> Result {co_return t; }();
                VERBOSE(std::cout << "constructed Result 2 at " << (u64)this << " " << *this << std::endl);
            }

            Result(const typename std::enable_if_t<std::is_copy_constructible<error_type>::value, error_type>& e)
            {
                *this = [&]() -> Result { co_return e; }();
                VERBOSE(std::cout << "constructed Result 3 at " << (u64)this << " " << *this << std::endl);
            }

            Result(typename std::enable_if_t<std::is_move_constructible<error_type>::value, error_type>&& e)
            {
                *this = [&]()-> Result {co_return e; }();
                VERBOSE(std::cout << "constructed Result 4 at " << (u64)this << " " << *this << std::endl);
            }

            Result(Result&& r)
            {
                var() = r;
                r.coroutine_handle = {};
                VERBOSE(std::cout << "constructed Result 5 at " << (u64)this << " " << *this << std::endl);
            }


            std::variant<value_type, error_type>& var() {
                return coroutine_handle.promise().mVar;
            };
            const std::variant<value_type, error_type>& var() const {
                return coroutine_handle.promise().mVar;
            };

#endif

            Result& operator=(const value_type& v)
            {
                var() = v;
                VERBOSE(std::cout << "assign Result 1 at " << (u64)this << " " << *this << std::endl);
                return *this;
            }


            Result& operator=(value_type&& v)
            {
                var() = std::move(v);
                VERBOSE(std::cout << "assign Result 2 at " << (u64)this << " " << *this << std::endl);
                return *this;
            }

            Result& operator=(Result&& r)
            {
                var() = std::move(r.var());
                return *this;
            }

            Result& operator=(Result const& r)
            {
                var() = r.var();
                return *this;
            }

            bool hasValue() const
            {
                return std::holds_alternative<value_type>(var());
            }

            bool hasError() const
            {
                return !hasValue();
            }

            operator bool() const
            {
                return hasError();
            }

            value_type& operator->()
            {
                return unwrap();
            }

            const value_type& operator->() const
            {
                return unwrap();
            }

            value_type& operator*()
            {
                return unwrap();
            }

            const value_type& operator*() const
            {
                return unwrap();
            }


            value_type& unwrap()
            {
                if (hasError())
                    exception_handler{}.throwErrorType(error());

                return std::get<value_type>(var());
            }

            const value_type& unwrap() const
            {
                if (hasError())
                    exception_handler{}.throwErrorType(error());

                return std::get<value_type>(var());
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
                return std::get<value_type>(var());
            }

            error_type& error()
            {
                if (hasValue())
                    throw BadResultAccess("error() was called on a Result<T,E> which stores an value_type");

                return std::get<error_type>(var());
            }

            const error_type& error() const
            {
                if (hasValue())
                    throw BadResultAccess("error() was called on a Result<T,E> which stores an value_type");

                return std::get<error_type>(var());
            }


            struct promise_type;


            template <class Promise>
            struct result_awaiter {

                ~result_awaiter()
                {
                    VERBOSE(std::cout << "~promise_type()" << std::endl);
                }
                using value_type = typename Promise::result_type::value_type;
                using exception_handler = typename  Promise::result_type::exception_handler;

                Result<value_type, error_type, exception_handler>&& res;

                result_awaiter(Result<value_type, error_type, exception_handler>&& r)
                    : res(std::move(r))
                {
                    VERBOSE(std::cout << "result_awaiter::result_awaiter()" << std::endl);
                }

                value_type&& await_resume() noexcept {
                    VERBOSE(std::cout << "result_awaiter::await_resume()" << std::endl);
                    return std::move(res.unwrap());
                }
                bool await_ready() noexcept {
                    VERBOSE(std::cout << "result_awaiter::await_ready()" << std::endl);
                    return res.hasValue();
                }

                void await_suspend(std::experimental::coroutine_handle<promise_type> const& handle) noexcept {
                    VERBOSE(std::cout << "result_awaiter::await_suspend()" << std::endl);
                    handle.promise().var() = res.error();
                }
            };

            struct promise_type
            {
                ~promise_type()
                {
                    VERBOSE(std::cout << "~promise_type()" << std::endl);
                }

                using result_type = Result;


                stde::suspend_never initial_suspend() {

                    VERBOSE(std::cout << "promise_type::initial_suspend()" << std::endl);
                    return {};
                }
#ifdef INLINE_VARIANT
                stde::suspend_never final_suspend() noexcept {
                    VERBOSE(std::cout << "promise_type::final_suspend()" << std::endl);
                    return {};
                }
#else
                stde::suspend_always final_suspend() noexcept {
                    VERBOSE(std::cout << "promise_type::final_suspend()" << std::endl);
                    return {};
                }
#endif
                Result get_return_object()
                {
                    VERBOSE(std::cout << "promise_type::get_return_object()" << std::endl);
                    return stde::coroutine_handle<promise_type>::from_promise(*this);
                }

                void return_value(value_type&& value) noexcept {
                    VERBOSE(std::cout << "result_promise::return_value(T&&)" << std::endl);
                    var() = std::move(value);
                }
                void return_value(value_type const& value) {
                    VERBOSE(std::cout << "result_promise::return_value(T const&)" << std::endl);
                    var() = value;
                }
                void return_value(std::error_code errc) noexcept {
                    VERBOSE(std::cout << "result_promise::return_value(E)" << std::endl);
                    var() = errc;
                }

                void unhandled_exception()
                {
                    VERBOSE(std::cout << "promise_type::unhandled_exception()" << std::endl);
                    var() = ExceptionHandler{}.unhandled_exception();
                }

                template <class TT, class EX>
                auto await_transform(Result<TT, error_type, EX>&& res) noexcept {
                    VERBOSE(std::cout << "result_promise::await_transform(Result<TT,E>&&)" << std::endl);
                    return result_awaiter<Result<TT, error_type, EX>::promise_type>(std::move(res));
                }
#ifdef INLINE_VARIANT
                std::variant<value_type, error_type>& var()
                {
                    return mRes->mVar;
                }
                result_type* mRes;
#else
                std::variant<value_type, error_type> mVar;
                std::variant<value_type, error_type>& var()
                {
                    return mVar;
                }
#endif
            };

            using coro_handle = std::experimental::coroutine_handle<promise_type>;
#ifdef INLINE_VARIANT
            std::variant<value_type, error_type> mVar;
#else
            coro_handle coroutine_handle;
#endif

            Result(coro_handle handle)
            {
#ifdef INLINE_VARIANT
                handle.promise().mRes = this;
#else
                coroutine_handle = handle;
#endif
                VERBOSE(std::cout << "constructed Result ** at " << (u64)this << " " << *this << std::endl);
            }
        };

        template<typename T, typename E, typename EX>
        std::ostream& operator<<(std::ostream& out, const Result<T, E, EX>& r)
        {
            if (r.hasValue())
                out << "{" << r.unwrap() << "}";
            else
                out << "<" << r.error().message() << ">" << std::endl;
            return out;
        }
    }

    template<typename T, typename E>
    struct RethrowExceptionHandler
    {
        std::variant<T, E> unhandled_exception()
        {
            throw;
        }

        void throwErrorType(E& e)
        {
            throw e;
        }
    };


    template<typename T, typename E>
    struct NothrowExceptionHandler;

    template<typename T>
    class NothrowExceptionHandler<T, std::error_code>
    {
    public:
        std::variant<T, std::error_code> unhandled_exception()
        {
            try 
            {
                throw;
            }
            catch (std::error_code e)
            {
                return e;
            }
            catch (...)
            {
                return Errc::uncaught_exception;
            }
            std::terminate();
        }

        void throwErrorType(std::error_code e)
        {
            throw e;
        }
    };

    template<typename T>
    class NothrowExceptionHandler<T, std::exception_ptr>
    {
    public:
        std::variant<T, std::exception_ptr> unhandled_exception()
        {
            return std::current_exception();
        }

        void throwErrorType(std::exception_ptr e)
        {
            std::rethrow_exception(e);
        }
    };


    template<typename T, typename E>
    class NothrowExceptionHandler
    {
    public:
        std::variant<T, E> unhandled_exception()
        {
            try {
                throw;
            }
            catch (E & e)
            {
                return e;
            }
            catch (...)
            {
                return E{};
            }
            std::terminate();
        }

        void throwErrorType(E const& e)
        {
            throw e;
        }
    };

    template<typename T, typename Error = std::error_code, typename ExceptionHandler = NothrowExceptionHandler<T, Error>>
    using Result = details::Result<T, Error, ExceptionHandler>;

}


int tupleMain();
//int tupleMain2();