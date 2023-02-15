#ifndef WF_OP_CODES_HPP
#define WF_OP_CODES_HPP

#include <cstdint>

namespace wf
{
    using Instruction = std::uint32_t;
    enum class OpCode : std::uint32_t
    {
        NO_OP,
        LOAD_CONSTANT,

        ADD_INT,
        SUBTRACT_INT,
        MULTIPLY_INT,
        DIVIDE_INT,
        MOD_INT,

        ADD_FLOAT,
        SUBTRACT_FLOAT,
        MULTIPLY_FLOAT,
        DIVIDE_FLOAT,
    };

    constexpr std::uint32_t OPCODE_MASK = 0b1111'1100'0000'0000'0000'0000'0000'0000;
    constexpr std::uint32_t OP_A_MASK   = 0b0000'0011'1111'1111'1110'0000'0000'0000;
    constexpr std::uint32_t OP_B_MASK   = 0b0000'0000'0000'0000'0001'1111'1111'1111;
}

#endif