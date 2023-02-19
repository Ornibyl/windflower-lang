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

        m_ip = m_state->stack.get_saved_ip();
        m_state->stack.pop_frame();
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
                    return;
                case Opcode::MOVE:
                    m_state->stack.index(instruction.get_op_a()) = m_state->stack.index(instruction.get_op_b());
                    break;
                case Opcode::LOAD_CONSTANT:
                    m_state->stack.index(instruction.get_op_a())
                        = m_state->stack.get_frame_function()->constants[instruction.get_op_b()];
                    break;
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
                case Opcode::MOD_INT:
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

    void Vm::error(const String& message)
    {
        throw VmError(format(m_state, "runtime error: {}", message));
    }
}