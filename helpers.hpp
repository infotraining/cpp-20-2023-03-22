#ifndef HELPERS_HPP
#define HELPERS_HPP

#include <string_view>
#include <iostream>

namespace Helpers
{
    void print(auto&& rng, std::string_view prefix = "items")
    {   
        std::cout << prefix << ": [ ";
        for(const auto& item : rng)
            std::cout << item << " ";
        std::cout << "]\n"; 
    }
}

#endif  //HELPERS_HPP