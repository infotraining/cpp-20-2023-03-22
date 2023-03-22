#include "../helpers.hpp"

#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <optional>
#include <string>
#include <utility>
#include <vector>

using namespace std::literals;

TEST_CASE("safe comparing integral numbers")
{
    int x = -7;
    unsigned int y = 665;
    // CHECK(x < y); // Error
    CHECK(std::cmp_less(x, y));

    auto cmp_gt = [](auto x, auto y) {
        return std::cmp_greater(x, y);
    };
    CHECK(cmp_gt(-7, 667u) == false);

    std::cout << std::in_range<std::size_t>(-1) << '\n';
    std::cout << std::in_range<std::size_t>(42) << '\n';
}

///////////////////////////////////////////////////////////////

namespace BeforeCpp20
{
    struct Value
    {
        int value;

        friend std::ostream& operator<<(std::ostream& out, const Value& v)
        {
            out << "Value{" << v.value << "}";
            return out;
        }

        [[nodiscard]] friend constexpr bool operator==(const Value& lhs, const Value& rhs) noexcept
        {
            return lhs.value == rhs.value;
        }

        [[nodiscard]] friend constexpr bool operator!=(const Value& lhs, const Value& rhs) noexcept
        {
            return !(lhs.value == rhs.value);
        }

        [[nodiscard]] friend constexpr bool operator<(const Value& lhs, const Value& rhs) noexcept
        {
            return lhs.value < rhs.value;
        }

        [[nodiscard]] friend constexpr bool operator<=(const Value& lhs, const Value& rhs) noexcept
        {
            return !(rhs.value < lhs.value);
        }

        [[nodiscard]] friend constexpr bool operator>(const Value& lhs, const Value& rhs) noexcept
        {
            return rhs.value < lhs.value;
        }

        [[nodiscard]] friend constexpr bool operator>=(const Value& lhs, const Value& rhs) noexcept
        {
            return !(lhs.value < rhs.value);
        }
    };
} // namespace BeforeCpp20

struct Value
{
    int value;

    constexpr Value(int v)
        : value{v}
    { }

    // bool operator==(const Value&) const = default; // implicit declared when operator<=> is defaulted

    std::strong_ordering operator<=>(const Value&) const = default;

    friend std::ostream& operator<<(std::ostream& out, const Value& v)
    {
        out << v.value;
        return out;
    }
};

struct Gadget
{
    std::string name; // std::strong_ordering
    double price;     // std::partial_ordering

    auto operator<=>(const Gadget&) const = default;
};

struct Id
{
    int id;

    auto operator<=>(const Id&) const = default;
};

struct Data
{
    int a, b, c;
    double factor;
    std::vector<int> data;
    Id id;

    auto operator<=>(const Data&) const = default;
};

TEST_CASE("operator ==")
{
    Value v1{42};
    Value v2 = 42;

    CHECK(v1 == v2);
    CHECK_FALSE(v1 != v2); // rewriting of expression !(v1 == v2)
    CHECK(v1 == 42);       // implicit conversion 42 -> Value{42}
    CHECK(42 == v1);       // rewriting to v1 == 42

    Gadget g1{"ipad", 5'000.0};
    Gadget g2{"ipad", 5'000.0};

    CHECK(g1 == g2);
    CHECK(g1 != Gadget{"ipod", 5'000});
    CHECK_FALSE(g1 > Gadget{"ipad", 7'000});
}

TEST_CASE("ordering with <, >, <=, >=")
{
    std::vector<Value> values = {3, 42, 665, 55, 9, -13, 45};
    std::sort(values.begin(), values.end());
    Helpers::print(values, "values asc");

    std::sort(values.begin(), values.end(), std::greater{});
    Helpers::print(values, "values desc");

    CHECK(Value{53} > 13);
    CHECK(Value{665} >= 55);
}

TEST_CASE("compare Data")
{
    Data d1{1, 2, 3, 3.14, {1, 2, 3}};
    Data d2{1, 2, 3, 3.14, {1, 2, 3}};

    CHECK(d1 <=> d2 == std::partial_ordering::equivalent);

    CHECK(d1 == d2);

    CHECK(d1 > Data{1, 2, 3, 2.71, {1, 2, 3}});
    CHECK(d1 < Data{1, 2, 3, 3.14, {1, 2, 7}});
}

TEST_CASE("operator <=>")
{
    SECTION("strong ordering")
    {
        int x = 42;

        CHECK(x <=> 42 == 0); // x == 42
        CHECK(x <=> 665 < 0); // x < 665
        CHECK(x <=> 22 > 0);  // x > 22

        std::strong_ordering result = 55 <=> 77;
        CHECK(result == std::strong_ordering::less);
        CHECK(77 <=> 77 == std::strong_ordering::equivalent);
        CHECK(665 <=> 100 == std::strong_ordering::greater);
    }

    SECTION("partial ordering")
    {
        CHECK(3.14 <=> 6.28 == std::partial_ordering::less);
        CHECK(3.14 <=> 3.14 == std::partial_ordering::equivalent);
        CHECK(0.0 <=> -0.0 == std::partial_ordering::equivalent);
        CHECK(std::numeric_limits<double>::quiet_NaN() <=> 5.66 == std::partial_ordering::unordered);
        CHECK(std::numeric_limits<double>::quiet_NaN() <=> 5.66 == std::partial_ordering::unordered);
        CHECK(std::numeric_limits<double>::quiet_NaN() <=> std::numeric_limits<double>::quiet_NaN() == std::partial_ordering::unordered);
    }
}

///////////////////////////////////////////////////////////
//

struct IntNan
{
    std::optional<int> value = std::nullopt;

public:
    bool operator==(const IntNan& rhs) const
    {
        if (!value || !rhs.value)
        {
            return false;
        }
        return *value == *rhs.value;
    }

    std::partial_ordering operator<=>(const IntNan& rhs) const
    {
        if (!value || !rhs.value)
            return std::partial_ordering::unordered;

        return *value <=> *rhs.value; // std::strong_ordering is implicitly converted to std::partial_ordering
    }
};

TEST_CASE("IntNan")
{
    CHECK(IntNan{2} <=> IntNan{4} == std::partial_ordering::less);
    CHECK(IntNan{2} <=> IntNan{} == std::partial_ordering::unordered);

    CHECK(IntNan{2} < IntNan{4});       // true
    CHECK_FALSE(IntNan{2} < IntNan{});  // false
    CHECK_FALSE(IntNan{2} == IntNan{}); // false
    CHECK_FALSE(IntNan{2} <= IntNan{}); // false
}

TEST_CASE("strong_order() & weak_order()")
{
    CHECK(std::strong_order(4.11, 8.66) == std::strong_ordering::less);
    CHECK(std::strong_order(std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN()) == std::strong_ordering::equal);
    CHECK(std::weak_order(std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN()) == std::weak_ordering::equivalent);
}

struct Temperature
{
    double value;

    std::strong_ordering operator<=>(const Temperature& temp) const
    {
        return std::strong_order(value, temp.value); // promotion from partial_ordering to strong_ordering for double
    }
};

TEST_CASE("promoted comparison category for Temperature")
{
    CHECK(Temperature{35.5} <=> Temperature{35.5} == std::strong_ordering::equal);
}

struct Person
{
    std::string first_name;
    std::string last_name;

    std::strong_ordering operator<=>(const Person& other) const // custom <=> operator
    {
        auto cmp_result = last_name <=> other.last_name;
        if (cmp_result != 0)
            return cmp_result;
        return first_name <=> other.first_name;
    }
};

template <typename T>
struct Wrapper
{
    T value;

    auto operator<=>(const Wrapper& v) const noexcept(noexcept(v <=> v))
    {
        return std::compare_three_way{}(value, v);
        // defines a total order for raw pointers
        // (which is not the case for operators <=> or <)
    }

    bool operator==(const Wrapper& other) const = default;
};

/////////////////////////////////////////////////////////////////////////////
namespace ver_1
{
    template <size_t N>
    struct DataSet
    {
        std::string name; // std::strong_ordering
        int values[N] = {};

        auto operator<=>(const DataSet&) const = default;
    };
} // namespace ver_1

template <size_t N>
struct DataSet
{
    std::string name; // std::strong_ordering
    int values[N] = {};

    auto operator==(const DataSet& rhs) const
    {
        // return std::equal(std::begin(values), std::end(values), std::begin(rhs.values));
        return std::ranges::equal(values, rhs.values);
    }

    auto operator<=>(const DataSet& rhs) const
    {
        return std::lexicographical_compare_three_way(std::begin(values), std::end(values), std::begin(rhs.values), std::end(rhs.values));
    }
};

TEST_CASE("compare DataSet")
{
    CHECK(DataSet{"ds2", {1, 2, 3}} == DataSet{"ds1", {1, 2, 3}});
    CHECK(DataSet{"ds1", {1, 2, 3}} < DataSet{"ds1", {1, 2, 6}});
    static_assert(std::is_same_v<decltype(std::declval<DataSet<2>>() <=> std::declval<DataSet<2>>()), std::strong_ordering>);
}