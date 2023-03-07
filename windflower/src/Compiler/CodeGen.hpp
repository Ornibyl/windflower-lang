#ifndef WF_CODE_GEN_HPP
#define WF_CODE_GEN_HPP

#include "Compiler/Actions.hpp"
#include "Compiler/Token.hpp"
#include "Vm/Object.hpp"
#include "Utils/HashMap.hpp"

namespace wf
{
    class CodeGen
    {
    public:
        CodeGen(State* state, BytecodeObject* output_code);

        void generate(const Action* action_tree);
    private:
        BytecodeObject* const m_output_code;
        std::uint32_t m_next_available_register = 0;
        std::uint32_t m_last_line = 0;

        HashMap<UInt, std::uint32_t> int_constant_map;
        HashMap<Float, std::uint32_t> float_constant_map;

        std::uint32_t push_constant(UInt value);
        std::uint32_t push_constant(Float value);

        void push_instruction(Opcode opcode, const SourcePosition& position);
        void push_instruction_two_op(Opcode opcode, std::uint32_t op_a, std::uint32_t op_b,
                const SourcePosition& position);
        void push_instruction_long_op(Opcode opcode, std::uint32_t operand,
                const SourcePosition& position);


        void gen_action(const Action* action);

        void gen_statement_block(const StatementBlockAction* action);
        void gen_create_stack_variable(const CreateStackVariableAction* action);
        void gen_return(const ReturnAction* action);

        void gen_int_binary_op(const IntBinaryAction* action);
        void gen_float_binary_op(const FloatBinaryAction* action);
        void gen_int_unary_op(const IntUnaryAction* action);
        void gen_float_unary_op(const FloatUnaryAction* action);
        void gen_numeric_conversion(const NumericConversionAction* action);
        void gen_int_constant(const IntConstantAction* action);
        void gen_float_constant(const FloatConstantAction* action);
        void gen_stack_variable_access(const StackVariableAccessAction* action);
    };
}

#endif