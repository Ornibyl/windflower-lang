#ifndef WF_OBJECT_HPP
#define WF_OBJECT_HPP

#include "Utils/Array.hpp"

namespace wf
{
    struct BytecodeLineInfo
    {
        std::size_t offset;
        std::uint16_t line;
    };

    struct Object
    {
        using polymorphic_size_enable = std::true_type;
        WF_POLYMORPHIC_SIZING

        enum class Type
        {
            BYTECODE,
        };

        virtual ~Object() = default;
    };

    struct BytecodeObject : public Object
    {
        WF_POLYMORPHIC_SIZING

        BytecodeObject(State* state)
            : line_info(state), code(state)
        {
        }

        DynamicArray<BytecodeLineInfo> line_info;
        DynamicArray<std::uint8_t> code;
    };
}

#endif