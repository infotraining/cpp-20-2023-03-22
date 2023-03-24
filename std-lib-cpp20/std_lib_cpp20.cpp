#include "../helpers.hpp"

#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <ranges>
#include <span>
#include <string>
#include <vector>
#include <bitset>

using namespace std::literals;

template <typename T, size_t N>
double avg(std::span<T, N> data)
{
    double sum = 0.0;
    for(const auto& item : data)
        sum += item;
    
    return sum / data.size();
}

TEST_CASE("std::span")
{
    int raw_array[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::vector vec = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    SECTION("fixed extent")
    {
        std::span<int, 5> sp1(raw_array, 5);
        std::span<int, 3> sp2(vec.begin() + 2, 3);

        Helpers::print(sp1, "sp1");
        Helpers::print(sp2, "sp2");

        std::ranges::fill(sp2, 0);
        Helpers::print(vec, "vec");

        std::span<int, 3> sp_evil{raw_array, 6}; // UB
    }

    SECTION("dynamic extent")
    {
        std::span<int> ds1(raw_array, 5);
        std::span<int> ds2(vec);

        Helpers::print(ds1, "ds1");
        Helpers::print(ds2, "ds2");

        auto subview = ds2.subspan(5, 3);
        Helpers::print(subview, "subview");
    }

    SECTION("sum")
    {
        std::cout << avg(std::span{raw_array}) << "\n";
        std::cout << avg(std::span{vec.begin(), 5}) << "\n";
    }
}

TEST_CASE("shallow constness")
{
    std::array data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    const std::span<int, 3> sp3 {data.begin(), 3};
    sp3[0] = 42;
    REQUIRE(data[0] == 42);

    std::span<const int, 3> reader_sp3(data.begin(), 3);
    // reader_sp3[0] = 1; // ERROR - span is read-only
}

void print_as_bytes(const float f, const std::span<const std::byte> bytes)
{
	std::cout << f << " - { ";

	for(const std::byte b : bytes)
	{
        std::bitset<8> bs(static_cast<uint8_t>(b));
		std::cout << bs << " ";
	}

	std::cout << "}\n";
}

TEST_CASE("span of bytes")
{
	float data[] = { 3.141592f };

	auto const const_bytes = std::as_bytes(std::span{data});
	print_as_bytes(data[0], const_bytes);

	auto const writable_bytes = std::as_writable_bytes(std::span{data});
	writable_bytes[3] |= std::byte{ 0b1000'0000 };
	print_as_bytes(data[0], writable_bytes);
}