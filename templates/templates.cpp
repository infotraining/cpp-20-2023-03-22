#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <numbers>

using namespace std::literals;

// NTTP

template <typename T, std::unsigned_integral auto N>
struct Array
{
    T values[N];
};

template <auto Factor>
auto scale_by(auto x)
{
    return Factor * x;
}

TEST_CASE("templates & lambda expressions")
{
    //Array<int, 100> arr1 = {};

    Array<int, 100UL> arr2 = {};

    CHECK(scale_by<3.14>(2.0) == 6.28);   
    std::cout << "2pi = " << scale_by<std::numbers::pi>(2.0) << "\n";
    std::cout << "2pi = " << scale_by<std::numbers::pi_v<float>>(2.0) << "\n";
}

////////////////////////////////////////
// structs as NTTP

struct Tax
{
    double value;

    constexpr Tax(double value) : value{value}
    {
        assert(value > 0 && value < 1.0);
    }
};

template <Tax Vat>
double calc_gross_price(double net_price)
{
    return net_price + net_price * Vat.value;
}

TEST_CASE("structs as NTTP")
{
    constexpr Tax vat_pl{0.23};
    constexpr Tax vat_ger{0.19};

    CHECK(calc_gross_price<vat_pl>(100.0) == 123.0);
    CHECK(calc_gross_price<vat_ger>(100.0) == 119.0);
}

//////////////////////////////////////////
// text as template parameter

template <size_t N>
struct Str
{
    char value[N];

    constexpr Str(const char (&str)[N])
    {
        std::copy(str, str + N, value);
    }

    friend std::ostream& operator<<(std::ostream& out, const Str& str)
    {
        out << str.value;

        return out;
    }
};

template <Str Prefix>
class Logger
{
public:
    void log(const std::string& msg)
    {
        std::cout << Prefix << msg << "\n";
    }
};  

TEST_CASE("Str as NTTP")
{
    Logger<">: "> logger1;
    logger1.log("Start");
    logger1.log("Stop");

    Logger<">>: "> logger2;
    logger2.log("Start");
    logger2.log("Stop");
}

///////////////////////////////////////////////////////////////
// Lambda as NTTP

template <std::invocable auto GetVat>
double calc_gross_price(double net_price)
{
    return net_price + net_price * GetVat();
}

TEST_CASE("Lambda as NTTP")
{
    CHECK(calc_gross_price<[] { return 0.23; }>(100.0) == 123.0);

    auto vat_ger = [] { return 0.19; };
    CHECK(calc_gross_price<vat_ger>(100.0) == 119.0);
}