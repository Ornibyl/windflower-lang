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

    template<typename T> requires(std::is_enum_v<T>)
    constexpr std::underlying_type_t<T> to_underlying(const T& value)
    {
        return static_cast<std::underlying_type_t<T>>(value);
    }
}

#endif