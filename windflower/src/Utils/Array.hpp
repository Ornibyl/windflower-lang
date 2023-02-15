#ifndef WF_ARRAY_HPP
#define WF_ARRAY_HPP

#include <vector>
#include <array>

#include "Allocate.hpp"

namespace wf
{
    template<typename T>
    using DynamicArray = std::vector<T, StdAllocator<T>>;

    template<typename T, std::size_t Size>
    using StaticArray = std::array<T, Size>;
}

#endif