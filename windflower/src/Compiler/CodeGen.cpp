#include "CodeGen.hpp"

namespace wf
{
    CodeGen::CodeGen(State* state, BytecodeObject* output_code)
        : m_output_code(output_code), int_constant_map(state), float_constant_map(state)
    {
    }

    void CodeGen::generate(const Action* action_tree)
    {
        std::uint32_t expr_position = m_next_available_register;
        gen_action(action_tree);
        push_instruction_long_op(Opcode::RETURN_VALUE, expr_position, SourcePosition::no_pos());
    }

    std::uint32_t CodeGen::push_constant(UInt value)
    {
        if(auto it = int_constant_map.find(value); it != int_constant_map.end())
        {
            return it->second;
        }

        std::uint32_t position = static_cast<std::uint32_t>(m_output_code->constants.size());
        int_constant_map[value] = position;

        m_output_code->constants.push_back(value);
        m_output_code->constant_type_infos.push_back(ConstantType::INT);
        return position;
    }

    std::uint32_t CodeGen::push_constant(Float value)
    {
        if(auto it = float_constant_map.find(value); it != float_constant_map.end())
        {
            return it->second;
        }

        std::uint32_t position = static_cast<std::uint32_t>(m_output_code->constants.size());
        float_constant_map[value] = position;

        m_output_code->constants.push_back(value);
        m_output_code->constant_type_infos.push_back(ConstantType::FLOAT);
        return position;
    }

    void CodeGen::push_instruction(Opcode opcode, const SourcePosition& position)
    {
        if(m_last_line != position.line && position != SourcePosition::no_pos())
        {
            m_output_code->line_info.emplace_back((BytecodeLineInfo){
                .offset = m_output_code->code.size(),
                .line = static_cast<std::uint16_t>(position.line)
            });
        }
        m_output_code->code.emplace_back(Instruction(opcode));
    }

    void CodeGen::push_instruction_two_op(Opcode opcode, std::uint32_t op_a, std::uint32_t op_b,
            const SourcePosition& position)
    {
        if(m_last_line != position.line && position != SourcePosition::no_pos())
        {
            m_output_code->line_info.emplace_back((BytecodeLineInfo){
                .offset = m_output_code->code.size(),
                .line = static_cast<std::uint16_t>(position.line)
            });
        }
        m_output_code->code.emplace_back(Instruction(opcode, op_a, op_b));
    }

    void CodeGen::push_instruction_long_op(Opcode opcode, std::uint32_t operand,
            const SourcePosition& position)
    {
        if(m_last_line != position.line && position != SourcePosition::no_pos())
        {
            m_output_code->line_info.emplace_back((BytecodeLineInfo){
                .offset = m_output_code->code.size(),
                .line = static_cast<std::uint16_t>(position.line)
            });
        }
        m_output_code->code.emplace_back(Instruction(opcode, operand));
    }


    void CodeGen::gen_action(const Action* action)
    {
        switch(action->type)
        {
            case Action::Type::INT_BINARY:
                gen_int_binary_op(static_cast<const IntBinaryAction*>(action));
                break;
            case Action::Type::FLOAT_BINARY:
                gen_float_binary_op(static_cast<const FloatBinaryAction*>(action));
                break;
            case Action::Type::INT_UNARY:
                gen_int_unary_op(static_cast<const IntUnaryAction*>(action));
                break;
            case Action::Type::FLOAT_UNARY:
                gen_float_unary_op(static_cast<const FloatUnaryAction*>(action));
                break;
            case Action::Type::NUMERIC_CONVERSION:
                gen_numeric_conversion(static_cast<const NumericConversionAction*>(action));
                break;
            case Action::Type::INT_CONSTANT:
                gen_int_constant(static_cast<const IntConstantAction*>(action));
                break;
            case Action::Type::FLOAT_CONSTANT:
                gen_float_constant(static_cast<const FloatConstantAction*>(action));
                break;
        }
    }

    void CodeGen::gen_int_binary_op(const IntBinaryAction* action)
    {
        const std::uint32_t left_operand_position = m_next_available_register;
        gen_action(action->get_left_operand());
        const std::uint32_t right_operand_position = m_next_available_register;
        gen_action(action->get_right_operand());

        Opcode opcode;

        switch(action->get_operation())
        {
            case IntBinaryOperation::ADD:
                opcode = Opcode::ADD_INT;
                break;
            case IntBinaryOperation::SUBTRACT:
                opcode = Opcode::SUBTRACT_INT;
                break;
            case IntBinaryOperation::MULTIPLY:
                opcode = Opcode::MULTIPLY_INT;
                break;
            case IntBinaryOperation::DIVIDE:
                opcode = Opcode::DIVIDE_INT;
                break;
            case IntBinaryOperation::MODULO:
                opcode = Opcode::MODULO_INT;
                break;
        }

        push_instruction_two_op(opcode, left_operand_position, right_operand_position, action->position);
        m_next_available_register--;
    }

    void CodeGen::gen_float_binary_op(const FloatBinaryAction* action)
    {
        const std::uint32_t left_operand_position = m_next_available_register;
        gen_action(action->get_left_operand());
        const std::uint32_t right_operand_position = m_next_available_register;
        gen_action(action->get_right_operand());

        Opcode opcode;

        switch(action->get_operation())
        {
            case FloatBinaryOperation::ADD:
                opcode = Opcode::ADD_FLOAT;
                break;
            case FloatBinaryOperation::SUBTRACT:
                opcode = Opcode::SUBTRACT_FLOAT;
                break;
            case FloatBinaryOperation::MULTIPLY:
                opcode = Opcode::MULTIPLY_FLOAT;
                break;
            case FloatBinaryOperation::DIVIDE:
                opcode = Opcode::DIVIDE_FLOAT;
                break;
        }

        push_instruction_two_op(opcode, left_operand_position, right_operand_position, action->position);
        m_next_available_register--;
    }

    void CodeGen::gen_int_unary_op(const IntUnaryAction* action)
    {
        const std::uint32_t operand_position = m_next_available_register;
        gen_action(action->get_operand());

        push_instruction_long_op(Opcode::NEGATION_INT, operand_position, action->position);
    }

    void CodeGen::gen_float_unary_op(const FloatUnaryAction* action)
    {
        const std::uint32_t operand_position = m_next_available_register;
        gen_action(action->get_operand());

        push_instruction_long_op(Opcode::NEGATION_FLOAT, operand_position, action->position);
    }

    void CodeGen::gen_numeric_conversion(const NumericConversionAction* action)
    {
        const std::uint32_t operand_position = m_next_available_register;
        gen_action(action->get_operand());

        if(action->get_from_type() == TypeId::INT
            && action->get_result_type() == TypeId::FLOAT)
        {
            push_instruction_long_op(Opcode::INT_TO_FLOAT, operand_position, action->position);
        }
        else if(action->get_from_type() == TypeId::FLOAT
            && action->get_result_type() == TypeId::INT)
        {
            push_instruction_long_op(Opcode::FLOAT_TO_INT, operand_position, action->position);
        }
    }

    void CodeGen::gen_int_constant(const IntConstantAction* action)
    {
        push_instruction_two_op(
            Opcode::LOAD_CONSTANT,
            m_next_available_register,
            push_constant(action->get_value()),
            action->position
        );

        m_next_available_register++;
    }

    void CodeGen::gen_float_constant(const FloatConstantAction* action)
    {
        push_instruction_two_op(
            Opcode::LOAD_CONSTANT,
            m_next_available_register,
            push_constant(action->get_value()),
            action->position
        );

        m_next_available_register++;
    }

}