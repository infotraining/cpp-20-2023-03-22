#include "../helpers.hpp"

#include <algorithm>
#include <array>
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <ranges>
#include <string>
#include <vector>

using namespace std::literals;

using namespace Helpers;

struct Value
{
    int v;

    auto operator<=>(const Value&) const = default;
};

TEST_CASE("algorithms")
{
    std::vector ds = create_numeric_dataset(20);
    print(ds, "ds");

    std::ranges::sort(ds);
    print(ds, "ds sorted");

    Value vs[] = {Value{6}, Value{2}, Value{42}};
    std::ranges::sort(vs);

    CHECK(std::ranges::is_sorted(vs));

    std::vector<std::string> words = {"zero", "one", "ten", "twenty", "two", "three", "four"};

    std::ranges::sort(words);
    print(words, "words");

    //                                                                                 // projection
    std::ranges::sort(std::ranges::begin(words), std::ranges::end(words), std::less{}, [](const auto& s) { return s.size(); });
    print(words, "words sorted by length");
}

template <auto Value_>
struct EndValue
{
    bool operator==(auto pos) const
    {
        return *pos == Value_;
    }
};

TEST_CASE("algorithms with sentinels")
{
    std::vector vec = {1, 2, 3, 4, 5, 42, 6, 7, 8, 9, 10};

    std::ranges::sort(vec.begin(), EndValue<42>{}, std::greater{});
    print(vec, "vec");

    // precondition: 42 is in range
    auto pos = std::ranges::find(vec.begin(), std::unreachable_sentinel, 42); // better performance
    CHECK(*pos == 42);

    auto head_vec = std::ranges::subrange(vec.begin(), EndValue<42>{});
    std::ranges::transform(head_vec, head_vec.begin(), [](int x) { return x * x; });

    print(head_vec, "head_vec");
}

TEST_CASE("views")
{
    std::vector ds = create_numeric_dataset(20);

    SECTION("all")
    {
        print(ds, "ds");

        auto all_ds = std::views::all(ds);

        print(all_ds, "all_ds");

        // auto evil = std::views::all(create_numeric_dataset(10));
        // print(evil, "evil");
    }

    SECTION("counted")
    {
        auto first_half = std::views::counted(ds.begin(), ds.size() / 2);
        print(first_half, "first_half");
    }

    SECTION("iota")
    {
        print(std::views::iota(1, 100), "numbers");
    }

    SECTION("creating views")
    {
        auto numbers = std::views::iota(1) | std::views::take(20);

        auto first_5 = std::views::take(std::views::transform(numbers, [](int x) { return x * x; }), 5);

        print(first_5, "first_5");
    }

    SECTION("piping")
    {
        auto numbers = std::views::iota(1, 20);

        auto first_5 = numbers
            | std::views::transform([](int x) { return x * x; })
            | std::views::filter([](int x) { return x % 2 == 0; })
            | std::views::take(5);

        print(first_5, "first_5");
    }

    SECTION("views - reference semantics")
    {
        std::vector ds = create_numeric_dataset(20);

        auto head_ds = ds | std::views::drop(10);

        for (auto& item : head_ds)
            item = 0;

        print(ds, "ds");
    }
}

namespace Views
{
    // template <typename T>
    // concept PrintableRange = std::ranges::range<T>
    //     && requires { std::cout << std::declval<std::ranges::range_value_t<T>>(); };

    // void print(PrintableRange auto&& rng, std::string_view prefix = "items")
    // {
    //     std::cout << prefix << ": [ ";
    //     for (const auto& item : rng)
    //         std::cout << item << " ";
    //     std::cout << "]\n";
    // }

    template <typename T>
    concept PrintableView = std::ranges::view<T>
        && requires(std::ranges::range_value_t<T> obj) { std::cout << obj; };
    void print(PrintableView auto rng, std::string_view prefix = "items")
    {
        std::cout << prefix << ": [ ";
        for (const auto& item : rng)
            std::cout << item << " ";
        std::cout << "]\n";
    }
} // namespace Views

TEST_CASE("passing views")
{
    std::vector ds = create_numeric_dataset(20);

    auto tail_ds = ds
        | std::views::drop(10)
        | std::views::filter([](int x) { return x % 2 == 0; });

    Views::print(tail_ds, "tail_ds");

    std::vector vec = {1, 2, 3, 4};
    Views::print(std::views::all(vec), "vec");
}

TEST_CASE("borrowed iterator")
{
    SECTION("dangling iterator")
    {
        auto pos = std::ranges::find(create_numeric_dataset(20, 1, 10), 6);

        // REQUIRE(*pos == 6); // Compilation ERROR - dangling iterator
    }

    SECTION("no dangling iterator")
    {
        auto&& temp = create_numeric_dataset(20, 1, 10);
        auto pos = std::ranges::find(temp, 6);

        REQUIRE(*pos == 6);
    }
}

///////////////////////////////////////////////////////
// Exercise

std::pair<std::string_view, std::string_view> split(const std::string_view& line, std::string_view separator = "/")
{
    std::pair<std::string_view, std::string_view> result;

    if (std::string::size_type pos = line.find(separator.data()); pos != std::string::npos)
    {
        result.first = std::string_view{line.begin(), line.begin() + pos};
        result.second = std::string_view{line.begin() + pos + 1, line.end()};
    }

    return result;
}

TEST_CASE("split")
{
    std::string s1 = "324/44";
    CHECK(split(s1) == std::pair{"324"sv, "44"sv});

    std::string s2 = "4343";
    CHECK(split(s2) == std::pair{""sv, ""sv});

    std::string s3 = "345/";
    CHECK(split(s3) == std::pair{"345"sv, ""sv});

    std::string s4 = "/434";
    CHECK(split(s3) == std::pair{""sv, "434"sv});
}

TEST_CASE("Exercise - ranges")
{
    const std::vector<std::string> lines = {
        "# Comment 1",
        "# Comment 2",
        "# Comment 3",
        "1/one",
        "2/two",
        "\n",
        "3/three",
        "4/four",
        "5/five",
        "\n",
        "\n",
        "6/six"
    };

    auto result = lines 
                    | std::views::drop_while([](const std::string& s) { return s.starts_with("#"); })
                    | std::views::filter([](const std::string &s){ return s != "\n"; })
                    | std::views::transform([](const std::string& s) { return split(s); })
                    | std::views::elements<1>;

    auto expected_result = {"one"s, "two"s, "three"s, "four"s, "five"s, "six"s};

    print(result, "result");

    CHECK(std::ranges::equal(result, expected_result));
}