#include "Result.h"


namespace { // anonymous namespace

    struct ncoroErrCategory : std::error_category
    {
        const char* name() const noexcept override
        {
            return "ncoro";
        }

        std::string message(int ev) const override
        {
            switch (static_cast<ncoro::Errc>(ev))
            {
            case ncoro::Errc::success:
                return "Success";

                // case osuCrypto::Errc::CloseChannel:
                //     return "the channel should be closed.";

            default:
                return "(unrecognized error)";
            }
        }
    };

    const ncoroErrCategory theNcoroCategory{};

} // anonymous namespace

namespace ncoro
{
    error_code ncoro::make_error_code(Errc e)
    {
        return { static_cast<int>(e), theNcoroCategory };
    }

}

template<typename T>
using Result = ncoro::Result<T>;


template<typename T>
using ResultE = ncoro::Result<T, std::exception_ptr, ncoro::NothrowExceptionHandler<T,std::exception_ptr>>;
// User code

int g_make_empty = 10;

const auto generic_error = std::error_code{ 1, std::system_category() };

// Classic style

Result<int> old_poll_value() {
    if (g_make_empty-- == 0) return { generic_error };
    return { 1 };
}

Result<long> old_sum_values(int nb_sum) {

    int sum = 0;
    while (nb_sum-- > 0) {
        auto res = old_poll_value();
        if (res) return res.error();
        sum += res.unwrap();
    }
    return { sum };
}

// Coro style

Result<int> poll_value() {
    if (g_make_empty-- == 0)
    {
        co_return generic_error;
    }
    co_return 1;
}

Result<long> sum_values(int nb_sum) {

    int sum = 0;
    while (nb_sum-- > 0) {
        sum += co_await poll_value();
    }
    co_return sum;
}

// exception based coro
Result<int> poll_value_ex() {
    if (g_make_empty-- == 0)
    {
        throw generic_error;
    }
    co_return 1;
}

Result<long> sum_values_ex(int nb_sum) {

    int sum = 0;
    while (nb_sum-- > 0) {
        sum += poll_value_ex().unwrap();
    }
    co_return sum;
}


// exception based coro
ResultE<int> poll_value_ex2() {
    if (g_make_empty-- == 0)
    {
        throw std::runtime_error("test rt");
    }
    co_return 1;
}

ResultE<long> sum_values_ex2(int nb_sum) {

    int sum = 0;
    while (nb_sum-- > 0) {
        sum += poll_value_ex().unwrap();
    }
    co_return sum;
}


template<typename R>
void print_opt(const R& r)
{
    std::cout << "res = " << r << '\n';
}

int tupleMain() {

    g_make_empty = 0;
    try {
        auto i = *poll_value_ex2();
    }
    catch (std::error_code const& e)
    {
        std::cout << "my ec catch " << e.message() << std::endl;
    }
    catch (std::runtime_error const&e)
    {
        std::cout << "my rt catch " << e.what() << std::endl;
    }
    catch (std::exception const& e)
    {
        std::cout << "my ex catch: " << e.what() << std::endl;
    }
    catch (...)
    {
        std::cout << "my unknown catch " << std::endl;
    }

    g_make_empty = 10;

    print_opt(sum_values_ex(5));
    print_opt(sum_values_ex(15));
    return 0;
}