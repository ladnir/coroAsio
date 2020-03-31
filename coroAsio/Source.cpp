//#include "Result.h"
//#include <experimental/coroutine>
//#include <variant>
//#include <optional>
//#include <system_error>
//#include <tuple>
//#include <iostream>
//#include <string>
//#include "cppcoro/task.hpp"
//
//
//namespace stde = std::experimental;
//
//namespace details2
//{
//
//    class Result
//    {
//    public:
//        using value_type = int;
//        using error_type = std::error_code;
//
//        ~Result()
//        {
//            std::cout << "~Result() " << (u64)this << std::endl;
//            if (coroutine_handle)
//                coroutine_handle.destroy();
//        }
//
//#ifdef INLINE_VARIANT
//        Result(const typename std::enable_if_t<std::is_copy_constructible<value_type>::value, value_type>& t)
//            :mVar(t)
//        {
//            std::cout << "constructed Result 1 at " << (u64)this << " " << *this << std::endl;
//        }
//
//        Result(typename std::enable_if_t<std::is_move_constructible<value_type>::value, value_type>&& t)
//            :mVar(std::forward<value_type>(t))
//        {
//            std::cout << "constructed Result 2 at " << (u64)this << " " << *this << std::endl;
//        }
//
//        Result(const typename std::enable_if_t<std::is_copy_constructible<error_type>::value, error_type>& e)
//            :mVar(e)
//        {
//            std::cout << "constructed Result 3 at " << (u64)this << " " << *this << std::endl;
//        }
//
//        Result(typename std::enable_if_t<std::is_move_constructible<error_type>::value, error_type>&& e)
//            :mVar(std::forward<error_type>(e))
//        {
//            std::cout << "constructed Result 4 at " << (u64)this << " " << *this << std::endl;
//        }
//
//        Result(Result&& r)
//        {
//            var() = r.mVar;
//            std::cout << "constructed Result 5 at " << (u64)this << " " << *this << std::endl;
//        }
//
//        std::variant<value_type, error_type>& var() {
//            return mVar;
//        };
//        const std::variant<value_type, error_type>& var() const {
//            return mVar;
//        };
//
//
//#else
//
//        Result(value_type const& t)
//        {
//            *this = [&]()-> Result {co_return t; }();
//            std::cout << "constructed Result 1 at " << (u64)this << " " << *this << std::endl;
//        }
//
//        Result(const error_type& e)
//        {
//            *this = [&]() -> Result { co_return e; }();
//            std::cout << "constructed Result 3 at " << (u64)this << " " << *this << std::endl;
//        }
//
//        Result(Result&& r)
//        {
//            coroutine_handle = r.coroutine_handle;
//            r.coroutine_handle = {};
//            std::cout << "constructed Result 5 at " << (long)this << " " << *this << std::endl;
//        }
//
//#endif
//
//        //Result& operator=(const value_type& v)
//        //{
//        //    var() = v;
//        //    std::cout << "assign Result 1 at " << (long)this << " " << *this << std::endl;
//        //    return *this;
//        //}
//
//
//        //Result& operator=(value_type&& v)
//        //{
//        //    var() = std::move(v);
//        //    std::cout << "assign Result 2 at " << (long)this << " " << *this << std::endl;
//        //    return *this;
//        //}
//
//        Result& operator=(Result&& r)
//        {
//            coroutine_handle = r.coroutine_handle;
//            r.coroutine_handle = nullptr;
//            return *this;
//        }
//
//        //Result& operator=(Result const& r)
//        //{
//        //    var() = r.var();
//        //    return *this;
//        //}
//
//        bool hasValue() const
//        {
//            return true;
//        }
//
//        bool hasError() const
//        {
//            return !hasValue();
//        }
//
//        operator bool() const
//        {
//            return hasError();
//        }
//
//        value_type& operator->()
//        {
//            return unwrap();
//        }
//
//        const value_type& operator->() const
//        {
//            return unwrap();
//        }
//
//        value_type& operator*()
//        {
//            return unwrap();
//        }
//
//        const value_type& operator*() const
//        {
//            return unwrap();
//        }
//
//
//        value_type& unwrap()
//        {
//            if (hasError())
//                throw BadResultAccess("unwrap() was called on a Result<T,E> which stores an error_type");
//
//            return *new value_type{};
//        }
//
//        const value_type& unwrap() const
//        {
//            if (hasError())
//                throw BadResultAccess("unwrap() was called on a Result<T,E> which stores an error_type");
//
//            return *new value_type{};
//        }
//
//        error_type& error()
//        {
//            if (hasValue())
//                throw BadResultAccess("error() was called on a Result<T,E> which stores an value_type");
//
//            return *new error_type{};
//        }
//
//        const error_type& error() const
//        {
//            if (hasValue())
//                throw std::runtime_error("error() was called on a Result<T,E> which stores an value_type");
//
//            return *new error_type{};
//        }
//
//
//        struct promise_type;
//
//
//        template <class Promise>
//        struct result_awaiter {
//
//            ~result_awaiter()
//            {
//                std::cout << "~promise_type()" << std::endl;
//            }
//
//
//            result_awaiter()
//            {
//                std::cout << "result_awaiter::result_awaiter()" << std::endl;
//            }
//
//            value_type& await_resume() noexcept {
//                std::cout << "result_awaiter::await_resume()" << std::endl;
//                return *new value_type{};
//            }
//            bool await_ready() noexcept {
//                std::cout << "result_awaiter::await_ready()" << std::endl;
//                return false;
//            }
//
//            void await_suspend(std::experimental::coroutine_handle<promise_type> const& handle) noexcept {
//                std::cout << "result_awaiter::await_suspend()" << std::endl;
//            }
//        };
//
//        struct promise_type
//        {
//            ~promise_type()
//            {
//                std::cout << "~promise_type()" << std::endl;
//            }
//
//            using result_type = Result;
//
//            //result_type* mRes;
//
//            stde::suspend_never initial_suspend() {
//
//                std::cout << "promise_type::initial_suspend()" << std::endl;
//                return {};
//            }
//            stde::suspend_always final_suspend() noexcept {
//                std::cout << "promise_type::final_suspend()" << std::endl;
//                return {};
//            }
//
//            Result get_return_object()
//            {
//                std::cout << "promise_type::get_return_object()" << std::endl;
//                return stde::coroutine_handle<promise_type>::from_promise(*this);
//            }
//
//            void return_value(value_type&& value) noexcept {
//                std::cout << "result_promise::return_value(T&&)" << std::endl;
//            }
//            void return_value(value_type const& value) {
//                std::cout << "result_promise::return_value(T const&)" << std::endl;
//            }
//            void return_value(std::error_code errc) noexcept {
//                std::cout << "result_promise::return_value(E)" << std::endl;
//            }
//
//            void unhandled_exception()
//            {
//                std::cout << "promise_type::unhandled_exception()" << std::endl;
//
//                auto ePtr = std::current_exception();
//                //return E{ 1,std::system_category() };
//                // rethrow the current exception;
//                if (ePtr)
//                {
//
//                    try
//                    {
//                        throw ePtr;
//                    }
//                    catch (std::error_code const& ec)
//                    {
//                        std::cout << "ec " << ec.message() << std::endl;
//                        throw;
//                    }
//                    catch (std::runtime_error & e)
//                    {
//                        std::cout << e.what() << '\n';  // or whatever
//                        throw;
//                    }
//                    catch (const std::exception & e)
//                    {
//                        std::cout << e.what() << '\n';  // or whatever
//                        throw;
//                    }
//                    catch (...)
//                    {
//                        // well ok, still unknown what to do now, 
//                        // but a std::exception_ptr doesn't help the situation either.
//                        std::cerr << "unknown exception\n";
//                        throw;
//                    }
//                }
//                throw std::exception();
//            }
//
//
//            auto await_transform(Result&& res) noexcept {
//                std::cout << "result_promise::await_transform(Result<TT,E>&&)" << std::endl;
//                return result_awaiter<Result::promise_type>();
//            }
//        };
//
//        using coro_handle = std::experimental::coroutine_handle<promise_type>;
//        coro_handle coroutine_handle;
//
//        Result(coro_handle handle)
//        {
//            coroutine_handle = handle;
//            std::cout << "constructed Result ** at " << (long)this << " " << *this << std::endl;
//
//        }
//
//    };
//
//}
//
//
//using Result = details2::Result;
//
//// User code
//
//int g_make_empty2 = 10;
//
//const auto generic_error = std::error_code{ 1, std::system_category() };
//
//// Classic style
//
//// exception based coro
//Result poll_value_ex() {
//    if (g_make_empty2-- == 0)
//    {
//        try
//        {
//            throw generic_error;
//        }
//        catch (std::runtime_error & rt)
//        {
//            std::cout << "rt" << rt.what() << std::endl;
//            throw;
//        }
//        catch (const std::error_code & ec)
//        {
//            std::cout << "ec" << ec.message() << std::endl;
//            throw;
//        }
//    }
//    co_return 1;
//}
//
//Result sum_values_ex(int nb_sum) {
//
//    int sum = 0;
//    while (nb_sum-- > 0) {
//        sum += poll_value_ex().unwrap();
//    }
//    co_return sum;
//}
//
//
//void print_opt(const Result& r)
//{
//    std::cout << "res = " << (r ? r.error().message() : std::to_string(r.unwrap())) << '\n';
//}
//
//int tupleMain2() {
//
//    g_make_empty2 = 0;
//    try
//    {
//        auto i = *poll_value_ex();
//    }
//    catch (const std::runtime_error & rt)
//    {
//        std::cout << rt.what() << std::endl;
//    }
//    catch (std::error_code const& ec)
//    {
//        std::cout << "ec " << ec.message() << std::endl;
//        throw;
//    }
//    catch (...)
//    {
//        std::cout << "unhandled exception" << std::endl;
//    }
//
//    return 0;
//}
