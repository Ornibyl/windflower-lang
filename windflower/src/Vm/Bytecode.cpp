#include "Bytecode.hpp"
#include "Utils/Format.hpp"

namespace wf
{
    constexpr std::uint8_t width = 10;

    static String register_operand(State* const state, std::uint32_t operand)
    {
        return format(state, "R({})", operand);
    }

    static String constant_operand(State* const state, std::uint32_t operand)
    {
        return format(state, "K({})", operand);
    }

    static String immediate_operand(State* const state, std::uint32_t operand)
    {
        return format(state, "I({})", operand);
    }

    static void write_constant_op(State* const state, String& result,
            std::string_view name, const Instruction& instruction)
    {
        format_to(result,
            "{1:<{0}}{2:<{0}}{3}\n", width, name,
                register_operand(state, instruction.get_op_a()),
                constant_operand(state, instruction.get_op_b())
        );
    }

    static void write_two_register_op(State* const state, String& result,
            std::string_view name, const Instruction& instruction)
    {
        format_to(result,
            "{1:<{0}}{2:<{0}}{3}\n", width, name,
                register_operand(state, instruction.get_op_a()),
                register_operand(state, instruction.get_op_b())
        );
    }

    static void write_one_register_op(State* const state, String& result,
            std::string_view name, const Instruction& instruction)
    {
        format_to(result,
            "{1:<{0}}{2}\n", width, name,
                register_operand(state, instruction.get_op_long())
        );
    }

    static void write_one_immediate_op(State* const state, String& result,
            std::string_view name, const Instruction& instruction)
    {
        format_to(result,
            "{1:<{0}}{2}\n", width, name,
                immediate_operand(state, instruction.get_op_long())
        );
    }

    static void write_no_operand_op(String& result, std::string_view name)
    {
        format_to(result, "{}\n", name);
    }

    String disassemble_bytecode_object(State* const state, BytecodeObject* code)
    {
        String result = format(state, "<***> Disassembled bytecode <***>\ncode:\n");

        for(const Instruction& instruction : code->code)
        {
            result += format(state, "    ");
            switch(instruction.get_opcode())
            {
                case Opcode::NO_OP:
                    write_no_operand_op(result, "nop");
                    break;
                case Opcode::RETURN:
                    write_no_operand_op(result, "nop");
                    break;
                case Opcode::RETURN_VALUE:
                    write_one_register_op(state, result, "retv", instruction);
                    break;
                case Opcode::RESERVE:
                    write_one_immediate_op(state, result, "rsv", instruction);
                    break;
                case Opcode::MOVE:
                    write_two_register_op(state, result, "mov", instruction);
                    break;
                case Opcode::LOAD_CONSTANT:
                    write_constant_op(state, result, "ldk", instruction);
                    break;
                case Opcode::NEGATION_INT:
                    write_one_register_op(state, result, "unmi", instruction);
                    break;
                case Opcode::NEGATION_FLOAT:
                    write_one_register_op(state, result, "unmf", instruction);
                    break;
                case Opcode::INT_TO_FLOAT:
                    write_one_register_op(state, result, "itof", instruction);
                    break;
                case Opcode::FLOAT_TO_INT:
                    write_one_register_op(state, result, "ftoi", instruction);
                    break;
                case Opcode::ADD_INT:
                    write_two_register_op(state, result, "addi", instruction);
                    break;
                case Opcode::SUBTRACT_INT:
                    write_two_register_op(state, result, "subi", instruction);
                    break;
                case Opcode::MULTIPLY_INT:
                    write_two_register_op(state, result, "muli", instruction);
                    break;
                case Opcode::DIVIDE_INT:
                    write_two_register_op(state, result, "divi", instruction);
                    break;
                case Opcode::MODULO_INT:
                    write_two_register_op(state, result, "modi", instruction);
                    break;
                case Opcode::ADD_FLOAT:
                    write_two_register_op(state, result, "addf", instruction);
                    break;
                case Opcode::SUBTRACT_FLOAT:
                    write_two_register_op(state, result, "subf", instruction);
                    break;
                case Opcode::MULTIPLY_FLOAT:
                    write_two_register_op(state, result, "mulf", instruction);
                    break;
                case Opcode::DIVIDE_FLOAT:
                    write_two_register_op(state, result, "divf", instruction);
                    break;
            }
        }

        if(code->constant_type_infos.size() != code->constants.size())
        {
            format_to(result, "data: {} constant(s)", code->constants.size());
            return result;
        }

        format_to(result, "data:\n");
        for(std::size_t i = 0; i < code->constants.size(); i++)
        {
            const Value& value = code->constants[i];
            ConstantType constant_type = code->constant_type_infos[i];

            format_to(result, "    K({})", i);

            switch(constant_type)
            {
                case ConstantType::INT:
                    format_to(result, "      : Int     = {}\n", value.as_int);
                    break;
                case ConstantType::FLOAT:
                    format_to(result, "      : Float   = {}\n", value.as_float);
                    break;
                case ConstantType::STRING:
                    format_to(result, "      : String  = \"{}\"\n", std::string_view(value.as_string()->text, value.as_string()->length));
                    break;
            }
        }

        return result;
    }
}