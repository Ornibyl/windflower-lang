#ifndef WF_FORMAT_HPP
#define WF_FORMAT_HPP

#include <fmt/format.h>

#include "Allocate.hpp"
#include "String.hpp"

namespace wf
{
    namespace detail
    {
        using MemoryBuffer = fmt::basic_memory_buffer<char, fmt::inline_buffer_size, StdAllocator<char>>;
    }

    inline String vformat(State* state, fmt::string_view format_str, fmt::format_args args)
    {
        detail::MemoryBuffer memory_buffer(state);
        fmt::vformat_to(std::back_inserter(memory_buffer), format_str, args);
        return String(memory_buffer.data(), memory_buffer.size(), state);
    }

    template<typename... Args>
    String format(State* state, fmt::format_string<Args...> format_str, Args&&... args)
    {
        return vformat(state, format_str, fmt::make_format_args(std::forward<Args>(args)...));
    }

    template<typename... Args>
    void format_to(String& result, fmt::format_string<Args...> format_str, Args&&... args)
    {
        fmt::format_to(std::back_inserter(result), format_str, std::forward<Args>(args)...);
    }
}

#endif