#include "../helpers.hpp"

#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <ranges>
#include <span>
#include <string>
#include <vector>
#include <bitset>
#include <memory>
#include <array>
#include <set>

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

///////////////////////////////////////////////////////////////
// Aggregates in C++20

struct Person
{
    int id;
    std::string name;
    int age;

    bool operator==(const Person& ) const = default;
};

TEST_CASE("designated initializers")
{
    Person p1{.id = 1, .age = 45};

    CHECK(p1 == Person{1, "", 45});

    //Person p2{.age = 55, .id = 2}; // designator order for field ‘Person::id’ does not match declaration order in ‘Person’
}

TEST_CASE("Init with ()")
{
    Person p1{1, "Kowalski", 44};
    Person p2(1, "Kowalski", 44);

    Person p3 = {2, "Nowak", 77};
    // Person p4 = (1, "Kowalski", 44); // ERROR

    auto ptr = std::make_unique<Person>(1, "John Pointer", 33);

    std::array arr1{1, 2, 3, 4, 5};
    // std::array arr2(1, 2, 3, 4, 5); // ERROR - {} required
}

template <typename T1, typename T2>
struct ValuePair
{
    T1 fst;
    T2 snd;
};

// template <typename T1, typename T2>
// ValuePair(T1, T2) -> ValuePair<T1, T2>;

TEST_CASE("CTAD")
{
    ValuePair vp1{1, "text"}; // ValuePair<int, const char*>
}


#include <bit>

//////////////////////////////////////////////////
// bit operations

template <typename T>
concept Integral = std::is_integral_v<T>;

template <size_t N>
concept PowerOf2 = std::has_single_bit(N);

template <typename T>
concept Addable = requires(T a, T b) { a + b; };

static_assert(PowerOf2<1024>);

template <typename TContainer, typename TValue>
    // klauzula
    requires    
        // wyrażenie requires
        requires { typename TContainer::value_type; }
void append(TContainer& container, TValue value)
{
    if constexpr(requires { container.push_back(value); })
    {
        container.push_back(value);
    }
    else
        container.insert(value);
}


template <typename T, std::unsigned_integral auto N>  
    requires PowerOf2<N>
struct Buffer
{
    T values[N];
};

TEST_CASE("bit operations")
{
    static_assert(std::has_single_bit(1024U));

    Buffer<std::byte, 1024U> buffer;

    std::set<int> mset;

    append(mset, 42);
}

///////////////////////////////////////////////////////////////
// Lambdas in C++20

TEST_CASE("lambda expressions")
{
    auto l1 = [](auto& coll, auto&& value) { coll.push_back(std::forward<decltype(value)>(value)); };

    auto l2 = []<typename T, typename TValue>(std::vector<T>& coll, TValue&& value) { coll.push_back(std::forward<TValue>(value)); };

    auto cmp_desc = [](const auto& a, const auto& b) { return a > b; };

    decltype(cmp_desc) another_cmp_desc; // new in C++20

    // std::set<int, decltype(cmp_desc)> mset_desc{{1, 2, 3, 4, 5, 6}, cmp_desc}; // Before C++20
    std::set<int, decltype(cmp_desc)> mset_desc = {1, 2, 3, 4, 5, 6} ; // Since C++20

    Helpers::print(mset_desc, "mset_desc");
}
