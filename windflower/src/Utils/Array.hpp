#ifndef WF_ARRAY_HPP
#define WF_ARRAY_HPP

#include <vector>
#include <array>
#include <initializer_list>

#include "Allocate.hpp"

namespace wf
{
    template<typename T>
    using DynamicArray = std::vector<T, StdAllocator<T>>;

    template<typename T, std::size_t Size>
    using StaticArray = std::array<T, Size>;

    template<typename T, std::size_t Size, typename KeyType> requires(std::is_integral_v<KeyType> || std::is_enum_v<KeyType>)
    constexpr StaticArray<T, Size> arr_from_designators(std::initializer_list<std::pair<KeyType, T>> init)
    {
        StaticArray<T, Size> array;

        for(const std::pair<KeyType, T>& designator : init)
        {
            array[static_cast<std::size_t>(designator.first)] = designator.second;
        }

        return array;
    }
}

#endif