#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <map>

using namespace std::literals;

void print(std::ranges::range auto rng, std::string_view prefix = "items")
{
    std::cout << prefix << ": [ ";
    for(const auto& item : rng)
        std::cout << item << " ";
    std::cout << "]\n";
}

TEST_CASE("constraints")
{
    print(std::vector{1, 2, 3}, "vec");
    REQUIRE(true);
}

TEST_CASE("concepts")
{
    REQUIRE(true);
}