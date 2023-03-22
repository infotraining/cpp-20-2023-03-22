#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <map>
#include <source_location>
#include <string>
#include <vector>
#include <complex>

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

TEST_CASE("constraints & concepts")
{
    int x = 7;
    int y = 42;
    CHECK(max_value(x, y) == 42);

    std::integral auto result_1 = max_value(x, y);
    //std::integral auto result_2 = max_value(6.7, 7.22); // the constraint was not satisfied
}

    CHECK(max_value(&x, &y) == 42);

    CHECK(max_value(std::string("abc"), std::string("def")) == "def"s);

    std::complex<double> c1{1.0, 5.4};
    std::complex<double> c2{6.7, 9.9};
    //CHECK(max_value(c1, c2)); // the constraint was not satisfied
}