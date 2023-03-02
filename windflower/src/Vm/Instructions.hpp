#ifndef WF_INSTRUCTIONS_HPP
#define WF_INSTRUCTIONS_HPP

#include <cstdint>

#include "Utils/Numeric.hpp"

namespace wf
{
    enum class Opcode : std::uint32_t
    {
        NO_OP, // nop

        RETURN, // ret
        RETURN_VALUE, // retv

        RESERVE, // rsv
        MOVE, // mov
        LOAD_CONSTANT, // ldk

        NEGATION_INT, // unmi
        NEGATION_FLOAT, // unmf

        INT_TO_FLOAT,
        FLOAT_TO_INT,

        ADD_INT, // addi
        SUBTRACT_INT, // subi
        MULTIPLY_INT, // muli
        DIVIDE_INT, // divi
        MODULO_INT, // modi

        ADD_FLOAT, // addf
        SUBTRACT_FLOAT, // subf
        MULTIPLY_FLOAT, // mulf
        DIVIDE_FLOAT, // divf
    };

    class Instruction
    {
    public:
        static constexpr std::uint32_t OPCODE_BIT_WIDTH = 8;
        static constexpr std::uint32_t OP_A_BIT_WIDTH = 12;
        static constexpr std::uint32_t OP_B_BIT_WIDTH = 12;
        static constexpr std::uint32_t OP_LONG_BIT_WIDTH = 24;
        static_assert(OPCODE_BIT_WIDTH + OP_A_BIT_WIDTH + OP_B_BIT_WIDTH == sizeof(std::uint32_t) * 8);
        static_assert(OPCODE_BIT_WIDTH + OP_LONG_BIT_WIDTH == sizeof(std::uint32_t) * 8);

        static constexpr std::uint32_t OPCODE_SHIFT = sizeof(std::uint32_t) * 8 - OPCODE_BIT_WIDTH;
        static constexpr std::uint32_t OP_A_SHIFT = sizeof(std::uint32_t) * 8 - OPCODE_BIT_WIDTH - OP_A_BIT_WIDTH;
        static constexpr std::uint32_t OP_B_SHIFT = sizeof(std::uint32_t) * 8 - OPCODE_BIT_WIDTH - OP_A_BIT_WIDTH - OP_B_BIT_WIDTH;
        static constexpr std::uint32_t OP_LONG_SHIFT = sizeof(std::uint32_t) * 8 - OPCODE_BIT_WIDTH - OP_LONG_BIT_WIDTH;

        static constexpr std::uint32_t OPCODE_MASK = filled_by_ones(OPCODE_BIT_WIDTH) << OPCODE_SHIFT;
        static constexpr std::uint32_t OP_A_MASK = filled_by_ones(OP_A_BIT_WIDTH) << OP_A_SHIFT;
        static constexpr std::uint32_t OP_B_MASK = filled_by_ones(OP_B_BIT_WIDTH) << OP_B_SHIFT;
        static constexpr std::uint32_t OP_LONG_MASK = filled_by_ones(OP_LONG_BIT_WIDTH) << OP_LONG_SHIFT;

        constexpr Instruction()
            : m_instruction(0)
        {
        }

        constexpr Instruction(Opcode opcode)
            : m_instruction(static_cast<std::uint32_t>(opcode) << OPCODE_SHIFT)
        {
        }

        constexpr Instruction(Opcode opcode, std::uint32_t op_a, std::uint32_t op_b)
            : m_instruction(
                (static_cast<std::uint32_t>(opcode) << OPCODE_SHIFT)
                    | (op_a << OP_A_SHIFT)
                    | (op_b << OP_B_SHIFT)
            )
        {
        }

        constexpr Instruction(Opcode opcode, std::uint32_t op_long)
            : m_instruction(
                (static_cast<std::uint32_t>(opcode) << OPCODE_SHIFT)
                    | (op_long << OP_LONG_SHIFT)
            )
        {
        }

        constexpr Opcode get_opcode() const noexcept
        {
            return Opcode{ (m_instruction & OPCODE_MASK) >> OPCODE_SHIFT };
        }

        constexpr std::uint32_t get_op_a() const noexcept
        {
            return (m_instruction & OP_A_MASK) >> OP_A_SHIFT;
        }

        constexpr std::uint32_t get_op_b() const noexcept
        {
            return (m_instruction & OP_B_MASK) >> OP_B_SHIFT;
        }

        constexpr std::uint32_t get_op_long() const noexcept
        {
            return (m_instruction & OP_LONG_MASK) >> OP_LONG_SHIFT;
        }

    private:
        std::uint32_t m_instruction;
    };
}

#endif