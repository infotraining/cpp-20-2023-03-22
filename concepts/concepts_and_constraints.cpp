#include <catch2/catch_test_macros.hpp>
#include <complex>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <source_location>
#include <string>
#include <string_view>
#include <vector>
#include <numeric>
#include "../helpers.hpp"

using namespace std::literals;

namespace Classics
{
    template <typename T>
    void foo(T arg)
    {
        auto src_loc = std::source_location::current();
        std::cout << src_loc.function_name() << "\n";
    }
} // namespace Classics

void foo(auto arg)
{
    auto src_loc = std::source_location::current();
    std::cout << src_loc.function_name() << "\n";
}

void print(auto&& rng, std::string_view prefix)
{
    std::cout << prefix << ": [ ";
    for (const auto& item : rng)
        std::cout << item << " ";
    std::cout << "]\n";
}

namespace Classics
{
    template <typename F, typename... TArgs>
    decltype(auto) call_wrapper(F&& f, TArgs&&... args)
    {
        std::cout << "call_wrapper!!!\n";
        return f(std::forward<TArgs>(args)...);
    }

    // template <typename F = int(*)(int, int), TArg1 = int, TArg2 = int>
    // auto call_wrapper(F&& f, TArg1&& arg1, TArg2&& arg2)
    // {
    //     std::cout << "call_wrapper!!!\n";
    //     return f(std::forward<TArg1>(arg1), std::forward<TArg2>(arg2));
    // }
} // namespace Classics

// athe same but with abbreviated template syntax
decltype(auto) call_wrapper(auto&& f, auto&&... args)
{
    auto src_loc = std::source_location::current();
    std::cout << src_loc.function_name() << "\n";

    return f(std::forward<decltype(args)>(args)...);
}

int multiply(int a, int b)
{
    return a * b;
}

int global_x = 42;

int& get_x()
{
    return global_x;
}

TEST_CASE("simplified templates")
{
    Classics::foo(42);
    foo(42);

    print(std::vector{1, 2, 3}, "vec");

    int result = call_wrapper(multiply, 2, 6);
    CHECK(result == 12);

    call_wrapper(get_x) = 665;

    CHECK(global_x == 665);
}

////////////////////////////////////////////////////////////
// type traits

template <typename T>
struct IsVoid
{
    constexpr static bool value = false;
};

template <>
struct IsVoid<void>
{
    constexpr static bool value = true;
};

template <typename T>
constexpr bool IsVoid_v = IsVoid<T>::value;

static_assert(IsVoid<int>::value == false);
static_assert(std::is_void<void>::value == true);
static_assert(IsVoid_v<int> == false);
static_assert(std::is_void_v<void> == true);

////////////////////////////////////////////////////////////

template <typename T>
struct IsPointer : std::false_type
{
};

template <typename T>
struct IsPointer<T*> : std::true_type
{
};

template <typename T>
constexpr bool IsPointer_v = IsPointer<T>::value;

static_assert(IsPointer_v<int> == false);
static_assert(IsPointer_v<int*> == true);

////////////////////////////////////////////////////////////
// constraints & concepts

namespace BeforeCpp20
{
    template <typename T, typename = std::enable_if_t<!IsPointer_v<T>>>
    T max_value(T a, T b)
    {
        return a < b ? b : a;
    }
} // namespace BeforeCpp20

// concept Pointer
template <typename T>
concept Pointer = IsPointer_v<T>;

static_assert(Pointer<int> == false);
static_assert(Pointer<const int*> == true);

namespace ver_1
{
    template <typename T>
        requires(!IsPointer_v<T>)
    T max_value(T a, T b)
    {
        return a < b ? b : a;
    }
} // namespace ver_1

namespace ver_2
{
    template <typename T>
        requires(!Pointer<T>)
    T max_value(T a, T b)
    {
        return a < b ? b : a;
    }

    template <typename T>
        requires(Pointer<T>)
    auto max_value(T a, T b)
    {
        assert(a != nullptr);
        assert(b != nullptr);

        return max_value(*a, *b);
    }
} // namespace ver_2

namespace ver_3
{
    template <typename T>
        requires(!Pointer<T>)
    T max_value(T a, T b)
    {
        return a < b ? b : a;
    }

    template <Pointer T>
    auto max_value(T a, T b)
    {
        assert(a != nullptr);
        assert(b != nullptr);

        return max_value(*a, *b);
    }
} // namespace ver_3

namespace ver_4
{
    template <typename T>
        requires(!Pointer<T> && std::three_way_comparable<T>)
    T max_value(T a, T b)
    {
        return a < b ? b : a;
    }

    auto max_value(Pointer auto a, Pointer auto b)
        requires std::three_way_comparable_with<decltype(*a), decltype(*b)> // trailing requires clause
    {
        assert(a != nullptr);
        assert(b != nullptr);

        return max_value(*a, *b);
    }
} // namespace ver_4

template <typename T>
concept LikePointer = requires(T ptr) {
                          *ptr;
                          ptr == nullptr;
                          ptr != nullptr;
                      };

template <typename T>
    requires(!LikePointer<T> && std::three_way_comparable<T>)
T max_value(T a, T b)
{
    return a < b ? b : a;
}

auto max_value(LikePointer auto a, LikePointer auto b)
    requires std::three_way_comparable_with<decltype(*a), decltype(*b)> // trailing requires clause
{
    assert(a != nullptr);
    assert(b != nullptr);

    return max_value(*a, *b);
}

TEST_CASE("constraints & concepts")
{
    int x = 7;
    int y = 42;
    CHECK(max_value(x, y) == 42);

    std::integral auto result_1 = max_value(x, y);
    // std::integral auto result_2 = max_value(6.7, 7.22); // the constraint was not satisfied

    CHECK(max_value(&x, &y) == 42);

    CHECK(max_value(std::string("abc"), std::string("def")) == "def"s);

     std::complex<double> c1{1.0, 5.4};
    // std::complex<double> c2{6.7, 9.9};
    // CHECK(max_value(c1, c2)); // the constraint was not satisfied

    auto sp_a = std::make_shared<int>(18);
    auto sp_b = std::make_shared<int>(42);
    CHECK(max_value(sp_a, sp_b) == 42);
}

/////////////////////////////////////////
// examples of concepts

template <typename T>
concept Hashable = requires(T a) {
                       { std::hash<T>{}(a) } -> std::convertible_to<std::size_t>;
                   };

static_assert(Hashable<int>);
//static_assert(Hashable<std::complex<double>>);

TEST_CASE("printable range")
{
    Helpers::print(std::vector{1, 2, 3}, "vec");
    //Helpers::print(std::map<int, std::string>{{1, "one"}}, "map");
}

template <typename T>
struct Holder
{
    T obj;

    void print() const requires Helpers::PrintableRange<T>
    {
        Helpers::print(obj, "items");
    }
};

TEST_CASE("Holder")
{
    Holder<int> h1{42};
    //h1.print();

    Holder<std::vector<int>> h2{std::vector{1, 2, 3}};
    h2.print();   
}

///////////////////////////////////////////////////
// exercise 1


auto is_power_of_2(std::integral auto value)
{
    return value > 0 && (value & (value - 1)) == 0;
}

template <std::floating_point T>
auto is_power_of_2(T value)
{
    int exponent;
    const T mantissa = std::frexp(value, &exponent);
    return mantissa == static_cast<T>(0.5);
}

TEST_CASE("is_power_of_2")
{
    REQUIRE(is_power_of_2(4));
    REQUIRE(is_power_of_2(8));
    REQUIRE(is_power_of_2(32));
    REQUIRE(is_power_of_2(77) == false);

    REQUIRE(is_power_of_2(8.0));
}

////////////////////////////////////////
// Catch with requires

template <typename T>
concept BadLargeIntegral = requires {
    std::is_integral_v<T>; // syntax is correct
    sizeof(T) >= 4; // syntax is correct
};

template <typename T>
concept LargeIntegral = std::integral<T> && requires {
    requires sizeof(T) >= 4;
};

static_assert(!LargeIntegral<char>);
static_assert(LargeIntegral<int>);
static_assert(!LargeIntegral<double>);


////////////////////////////////////////
// compound requirements

template <typename T>
concept Indexable = requires (T obj, size_t n) {
    { obj[n] } -> std::same_as<typename T::reference>;
    { obj.at(n) } -> std::same_as<typename T::reference>;
    { obj.size() } noexcept -> std::convertible_to<size_t>;
    { obj.~T() } noexcept;
};

template <typename T>
concept AdditiveRange = requires (T&& c) {
    std::ranges::begin(c);
    std::ranges::end(c); 
    typename std::ranges::range_value_t<T>; // type requirement
    requires requires(std::ranges::range_value_t<T> x) { x + x; }; // nested requirement
};
 
template <AdditiveRange Rng>
auto sum(const Rng& data)
{
    return std::accumulate(std::begin(data), std::end(data), 
        std::ranges::range_value_t<Rng>{});
}

TEST_CASE("AdditiveRange")
{ 
    assert(sum(std::vector{1, 2, 3}) == 6);
    //assert(sum(std::vector{ "one", "two", "three" }) == "onetwothree"s);
}

//////////////////////////////////////////////////
// Exercise 2

TEST_CASE("StdContainer concept")
{
    static_assert(Indexable<int[10]>);

    static_assert(StdContainer<std::vector<int>>);
    static_assert(StdContainer<std::list<int>>);
    static_assert(StdContainer<std::string>);
    static_assert(StdContainer<std::string_view>);
    static_assert(!StdContainer<int[10]>);

    static_assert(IndexableContainer<std::vector<int>>);
    static_assert(!IndexableContainer<std::list<int>>);
    static_assert(IndexableContainer<std::map<int, std::string>>);
}