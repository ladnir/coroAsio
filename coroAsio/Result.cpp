#include "Result.h"
#include <experimental/coroutine>
#include <variant>
#include <optional>
#include <system_error>
#include <tuple>
#include <iostream>
#include <string>
#include "cppcoro/task.hpp"


namespace stde = std::experimental;
std::exception_ptr myEPtr = nullptr;
std::runtime_error* myRt = nullptr;
namespace details
{
    struct DefaultResultExceptionHandler
    {
        template<typename T, typename E>
        std::variant<T, E> unhandled_exception()
        {
            auto ePtr = std::current_exception();
            //return E{ 1,std::system_category() };
            // rethrow the current exception;
            if (ePtr)
            {

                try
                {
                    throw ePtr;
                }
                catch (std::error_code const& ec)
                {
                    std::cout << "ec " << ec.message() << std::endl;
                    throw;
                }
                catch (std::runtime_error & e)
                {
                    std::cout << e.what() << '\n';  // or whatever

                }
                catch (const std::exception & e)
                {
                    std::cout << e.what() << '\n';  // or whatever
                }
                catch (...)
                {
                    // well ok, still unknown what to do now, 
                    // but a std::exception_ptr doesn't help the situation either.
                    std::cerr << "unknown exception\n";
                    throw;
                }

                std::cout << "eq " << int(myEPtr == ePtr) << std::endl;

            }
            throw std::exception();
        }
    };


    template<typename T, typename Error, typename ExceptionHandler>
    class Result
    {
    public:
        using value_type = std::remove_cvref_t<T>;
        using error_type = std::remove_cvref_t<Error>;
        using exception_handler = std::remove_cvref_t<ExceptionHandler>;

        ~Result()
        {
            std::cout << "~Result() " << (long)this << std::endl;
            if (coroutine_handle)
                coroutine_handle.destroy();
        }

#ifdef INLINE_VARIANT
        Result(const typename std::enable_if_t<std::is_copy_constructible<value_type>::value, value_type>& t)
            :mVar(t)
        {
            std::cout << "constructed Result 1 at " << (long)this << " " << *this << std::endl;
        }

        Result(typename std::enable_if_t<std::is_move_constructible<value_type>::value, value_type>&& t)
            :mVar(std::forward<value_type>(t))
        {
            std::cout << "constructed Result 2 at " << (long)this << " " << *this << std::endl;
        }

        Result(const typename std::enable_if_t<std::is_copy_constructible<error_type>::value, error_type>& e)
            :mVar(e)
        {
            std::cout << "constructed Result 3 at " << (long)this << " " << *this << std::endl;
        }

        Result(typename std::enable_if_t<std::is_move_constructible<error_type>::value, error_type>&& e)
            :mVar(std::forward<error_type>(e))
        {
            std::cout << "constructed Result 4 at " << (long)this << " " << *this << std::endl;
        }

        Result(Result&& r)
        {
            var() = r.mVar;
            std::cout << "constructed Result 5 at " << (long)this << " " << *this << std::endl;
        }

        std::variant<value_type, error_type>& var() {
            std::cout << "var() " << (long)this << std::endl;

            return mVar;
        };
        const std::variant<value_type, error_type>& var() const {
            std::cout << "var() " << (long)this << std::endl;

            return mVar;
        };


#else

        Result(const typename std::enable_if_t<std::is_copy_constructible<value_type>::value, value_type>& t)
        {
            *this = [&]()-> Result {co_return t; }();
            std::cout << "constructed Result 1 at " << (long)this << " " << *this << std::endl;
        }

        Result(typename std::enable_if_t<std::is_move_constructible<value_type>::value, value_type>&& t)
        {
            *this = [&]()-> Result {co_return t; }();
            std::cout << "constructed Result 2 at " << (long)this << " " << *this << std::endl;
        }

        Result(const typename std::enable_if_t<std::is_copy_constructible<error_type>::value, error_type>& e)
        {
            *this = [&]() -> Result { co_return e; }();
            std::cout << "constructed Result 3 at " << (long)this << " " << *this << std::endl;
        }

        Result(typename std::enable_if_t<std::is_move_constructible<error_type>::value, error_type>&& e)
        {
            *this = [&]()-> Result {co_return e; }();
            std::cout << "constructed Result 4 at " << (long)this << " " << *this << std::endl;
        }

        Result(Result&& r)
        {
            var() = r;
            r.coroutine_handle = {};
            std::cout << "constructed Result 5 at " << (long)this << " " << *this << std::endl;
        }


        std::variant<value_type, error_type>& var() {
            std::cout << "var() " << (long)this << std::endl;

            return coroutine_handle.promise().mVar;
        };
        const std::variant<value_type, error_type>& var() const {
            std::cout << "var() " << (long)this << std::endl;

            return coroutine_handle.promise().mVar;
        };

#endif

        Result& operator=(const value_type& v)
        {
            var() = v;
            std::cout << "assign Result 1 at " << (long)this << " " << *this << std::endl;
            return *this;
        }


        Result& operator=(value_type&& v)
        {
            var() = std::move(v);
            std::cout << "assign Result 2 at " << (long)this << " " << *this << std::endl;
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
                throw std::runtime_error("unwrap() was called on a Result<T,E> which stores an error_type");

            return std::get<value_type>(var());
        }

        const value_type& unwrap() const
        {
            if (hasError())
                throw std::runtime_error("unwrap() was called on a Result<T,E> which stores an error_type");

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
                throw std::runtime_error("error() was called on a Result<T,E> which stores an value_type");

            return std::get<error_type>(var());
        }

        const error_type& error() const
        {
            if (hasValue())
                throw std::runtime_error("error() was called on a Result<T,E> which stores an value_type");

            return std::get<error_type>(var());
        }


        struct promise_type;


        template <class Promise>
        struct result_awaiter {

            ~result_awaiter()
            {
                std::cout << "~promise_type()" << std::endl;
            }
            using value_type = typename Promise::result_type::value_type;
            using exception_handler = typename  Promise::result_type::exception_handler;

            Result<value_type, error_type, exception_handler>&& res;

            result_awaiter(Result<value_type, error_type, exception_handler>&& r) :res(std::move(r))
            {
                std::cout << "result_awaiter::result_awaiter()" << std::endl;
            }

            value_type&& await_resume() noexcept {
                std::cout << "result_awaiter::await_resume()" << std::endl;
                return std::move(res.unwrap());
            }
            bool await_ready() noexcept {
                std::cout << "result_awaiter::await_ready()" << std::endl;
                return res.hasValue();
            }

            void await_suspend(std::experimental::coroutine_handle<promise_type> const& handle) noexcept {
                std::cout << "result_awaiter::await_suspend()" << std::endl;
                handle.promise().var() = res.error();
            }
        };

        struct promise_type
        {
            ~promise_type()
            {
                std::cout << "~promise_type()" << std::endl;
            }

            using result_type = Result;

            //result_type* mRes;

            stde::suspend_never initial_suspend() { return {}; }
            stde::suspend_always final_suspend() noexcept { return {}; }

            Result get_return_object()
            {
                return stde::coroutine_handle<promise_type>::from_promise(*this);
            }

            void return_value(value_type&& value) noexcept {
                std::cout << "result_promise::return_value(T&&)" << std::endl;
                var() = std::move(value);
            }
            void return_value(value_type const& value) {
                std::cout << "result_promise::return_value(T const&)" << std::endl;
                var() = value;
            }
            void return_value(std::error_code errc) noexcept {
                std::cout << "result_promise::return_value(E)" << std::endl;
                var() = errc;
            }

            void unhandled_exception()
            {
                var() = ExceptionHandler{}.unhandled_exception<value_type, error_type>();
            }

            template <class TT, class EX>
            auto await_transform(Result<TT, error_type, EX>&& res) noexcept {
                std::cout << "result_promise::await_transform(Result<TT,E>&&)" << std::endl;
                return result_awaiter<Result<TT, error_type, EX>::promise_type>(std::move(res));
            }
#ifdef INLINE_VARIANT
            std::variant<value_type, error_type>& var()
            {
                std::cout << "var() " << (long)mRes << std::endl;
                return mRes->mVar;
            }
            result_type* mRes;
#else
            std::variant<value_type, error_type> mVar;
            std::variant<value_type, error_type>& var()
            {
                //std::cout << "var() " << (long)mRes << std::endl;
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
            std::cout << "constructed Result ** at " << (long)this << " " << *this << std::endl;

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


struct FancyResultExceptionHandler
{
    template<typename T, typename E>
    std::variant<T, E> unhandled_exception()
    {
        return E{ 1,std::system_category() };
    }
};

template<typename T, typename Error = std::error_code, typename ExceptionHandler = details::DefaultResultExceptionHandler>
using Result = details::Result<T, Error, ExceptionHandler>;

// User code

int g_make_empty = 10;

const auto generic_error = std::error_code{ 1, std::system_category() };

// Classic style

Result<int, std::error_code> old_poll_value() {
    if (g_make_empty-- == 0) return { generic_error };
    return { 1 };
}

Result<long, std::error_code> old_sum_values(int nb_sum) {

    int sum = 0;
    while (nb_sum-- > 0) {
        auto res = old_poll_value();
        if (res) return res.error();
        sum += res.unwrap();
    }
    return { sum };
}

// Coro style

Result<int, std::error_code> poll_value() {
    if (g_make_empty-- == 0)
    {
        co_return generic_error;
    }
    co_return 1;
}

Result<long, std::error_code> sum_values(int nb_sum) {

    int sum = 0;
    while (nb_sum-- > 0) {
        sum += co_await poll_value();
    }
    co_return sum;
}

// exception based coro
Result<int, std::error_code> poll_value_ex() {
    if (g_make_empty-- == 0)
    {
        try
        {
            throw generic_error;
        }
        catch (std::runtime_error & rt)
        {
            myRt = &rt;
            myEPtr = std::current_exception();
            throw;
        }
        catch (const std::error_code & ec)
        {
            std::cout << "ec" << ec.message() << std::endl;
            throw;
        }
    }
    co_return 1;
}

Result<long, std::error_code> sum_values_ex(int nb_sum) {

    int sum = 0;
    while (nb_sum-- > 0) {
        sum += poll_value_ex().unwrap();
    }
    co_return sum;
}


template<typename T, typename E>
void print_opt(const Result<T, E>& r)
{
    std::cout << "res = " << (r ? r.error().message() : std::to_string(r.unwrap())) << '\n';
}

int tupleMain() {

    g_make_empty = 0;
    try
    {
        auto i = *poll_value_ex();
    }
    catch (const std::runtime_error & rt)
    {

        std::cout << "eq " << int(myEPtr == std::current_exception()) << std::endl;
        std::cout << "eq " << int(myRt == &rt) << std::endl;

        std::cout << rt.what() << std::endl;
    }
    catch (std::error_code const& ec)
    {
        std::cout << "ec " << ec.message() << std::endl;
        throw;
    }
    catch (...)
    {
        std::cout << "unhandled exception" << std::endl;
    }

    print_opt(sum_values_ex(5));
    print_opt(sum_values_ex(15));
    return 0;
}
