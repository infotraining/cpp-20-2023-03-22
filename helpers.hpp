#ifndef HELPERS_HPP
#define HELPERS_HPP

#include <iostream>
#include <string_view>
#include <random>
#include <ranges>
#include <algorithm>

namespace Helpers
{
    template <typename T>
    concept PrintableRange = std::ranges::range<T>
        && requires { std::cout << std::declval<std::ranges::range_value_t<T>>(); };

    void print(PrintableRange auto&& rng, std::string_view prefix = "items")
    {
        std::cout << prefix << ": [ ";
        for (const auto& item : rng)
            std::cout << item << " ";
        std::cout << "]\n";
    }

    std::vector<int> create_numeric_dataset(size_t size, int low = -100, int high = 100)
    {
        std::vector<int> data(size);

        std::mt19937 rnd_gen{42};
        std::uniform_int_distribution<> distr(low, high);

        std::ranges::generate(data, [&]{ return distr(rnd_gen);});

        return data;
    }
} // namespace Helpers

#endif // HELPERS_HPP