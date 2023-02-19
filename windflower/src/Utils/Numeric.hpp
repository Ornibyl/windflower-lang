#ifndef WF_NUMERIC_HPP
#define WF_NUMERIC_HPP

#include <cstdint>
#include <concepts>

namespace wf
{
    template<typename T> requires std::integral<T>
    constexpr T filled_by_ones(T ones_count)
    {
        return (static_cast<T>(1) << ones_count) - static_cast<T>(1);
    }
}

#endif