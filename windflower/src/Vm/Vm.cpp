#include "Vm.hpp"

#include "State.hpp"
#include "Utils/Format.hpp"

namespace wf
{
    void Vm::call(std::size_t idx, std::size_t return_idx)
    {
        BytecodeObject* function = m_state->stack.index(idx).as_bytecode();
        m_state->stack.push_frame(function, m_ip, return_idx);

        run();
    }

    void Vm::run()
    {
        m_ip = 0;
        while(true)
        {
            Instruction instruction = m_state->stack.get_frame_function()->code[m_ip++];

            switch(instruction.get_opcode())
            {
                case Opcode::NO_OP:
                    break;
                case Opcode::RESERVE:
                    m_state->stack.reserve(instruction.get_op_long());
                    break;
                case Opcode::RETURN:
                    m_ip = m_state->stack.get_saved_ip();
                    m_state->stack.pop_frame();
                    return;
                case Opcode::RETURN_VALUE:
                {
                    const std::size_t saved_return_idx = m_state->stack.get_return_idx();
                    const Value return_value = m_state->stack.index(instruction.get_op_long());

                    m_ip = m_state->stack.get_saved_ip();
                    m_state->stack.pop_frame();
                    m_state->stack.index(saved_return_idx) = return_value;
                    return;
                }
                case Opcode::MOVE:
                    m_state->stack.index(instruction.get_op_a()) = m_state->stack.index(instruction.get_op_b());
                    break;
                case Opcode::LOAD_CONSTANT:
                    m_state->stack.index(instruction.get_op_a())
                        = m_state->stack.get_frame_function()->constants[instruction.get_op_b()];
                    break;
                case Opcode::NEGATION_INT:
                {
                    Value& value = m_state->stack.index(instruction.get_op_long());
                    value.as_int = -static_cast<UInt>(static_cast<Int>(value.as_int));
                    break;
                }
                case Opcode::NEGATION_FLOAT:
                {
                    Value& value = m_state->stack.index(instruction.get_op_long());
                    value.as_float = -value.as_float;
                    break;
                }
                case Opcode::INT_TO_FLOAT:
                {
                    Value& value = m_state->stack.index(instruction.get_op_long());
                    value.as_float = static_cast<Float>(static_cast<Int>(value.as_int));
                    break;
                }
                case Opcode::FLOAT_TO_INT:
                {
                    Value& value = m_state->stack.index(instruction.get_op_long());
                    value.as_int = static_cast<UInt>(static_cast<Int>(value.as_float));
                    break;
                }
                case Opcode::ADD_INT:
                    m_state->stack.index(instruction.get_op_a()).as_int
                        += m_state->stack.index(instruction.get_op_b()).as_int;
                    break;
                case Opcode::SUBTRACT_INT:
                    m_state->stack.index(instruction.get_op_a()).as_int
                        -= m_state->stack.index(instruction.get_op_b()).as_int;
                    break;
                case Opcode::MULTIPLY_INT:
                    m_state->stack.index(instruction.get_op_a()).as_int
                        *= m_state->stack.index(instruction.get_op_b()).as_int;
                    break;
                case Opcode::DIVIDE_INT:
                {
                    const Value& rhs_value = m_state->stack.index(instruction.get_op_b());

                    if(rhs_value.as_int == 0)
                    {
                        error(String("Cannot divide an integer by 0.", m_state));
                    }

                    m_state->stack.index(instruction.get_op_a()).as_int
                        /= rhs_value.as_int;
                    break;
                }
                case Opcode::MODULO_INT:
                {
                    const Value& rhs_value = m_state->stack.index(instruction.get_op_b());

                    if(rhs_value.as_int == 0)
                    {
                        error(String("Cannot divide an integer by 0.", m_state));
                    }

                    m_state->stack.index(instruction.get_op_a()).as_int
                        %= rhs_value.as_int;
                    break;
                }
                case Opcode::ADD_FLOAT:
                    m_state->stack.index(instruction.get_op_a()).as_float
                        += m_state->stack.index(instruction.get_op_b()).as_float;
                    break;
                case Opcode::SUBTRACT_FLOAT:
                    m_state->stack.index(instruction.get_op_a()).as_float
                        -= m_state->stack.index(instruction.get_op_b()).as_float;
                    break;
                case Opcode::MULTIPLY_FLOAT:
                    m_state->stack.index(instruction.get_op_a()).as_float
                        *= m_state->stack.index(instruction.get_op_b()).as_float;
                    break;
                case Opcode::DIVIDE_FLOAT:
                    m_state->stack.index(instruction.get_op_a()).as_float
                        /= m_state->stack.index(instruction.get_op_b()).as_float;
                    break;
            }
        }
    }

    std::uint16_t Vm::get_current_line() const
    {
        std::size_t current_line_offset = 0;
        std::uint16_t current_line = 0;
        for(const BytecodeLineInfo& line_info : m_state->stack.get_frame_function()->line_info)
        {
            if(line_info.offset < m_ip && line_info.offset > current_line_offset)
            {
                current_line_offset = line_info.offset;
                current_line = line_info.line;
            }
        }

        return current_line;
    }

    void Vm::error(const String& message)
    {
        const std::uint16_t current_line = get_current_line();

        if(current_line == 0)
        {
            throw VmError(format(m_state, "runtime error(\?\?\?): {}", current_line, message));
        }
        else
        {
            throw VmError(format(m_state, "runtime error(ln: {}): {}", current_line, message));
        }
    }
}