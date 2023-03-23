#ifndef HELPERS_HPP
#define HELPERS_HPP

#include <iostream>
#include <string_view>

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
} // namespace Helpers

#endif // HELPERS_HPP