#ifndef WF_ACTIONS_HPP
#define WF_ACTIONS_HPP

#include "Compiler/SymbolTable.hpp"
#include "Compiler/Token.hpp"
#include "Utils/Allocate.hpp"
#include "Utils/Array.hpp"
#include "Windflower/Windflower.hpp"

namespace wf
{
    #define WF_NUMERIC_UNARY_OPERATIONS ADD, SUBTRACT, MULTIPLY, DIVIDE

    constexpr std::string_view type_id_to_string(TypeId type_id)
    {
        switch(type_id)
        {
            case TypeId::VOID: return "Void";
            case TypeId::INT: return "Int";
            case TypeId::FLOAT: return "Float";
        }
    }

    constexpr bool type_id_is_numeric(TypeId type_id)
    {
        return type_id == TypeId::INT || type_id == TypeId::FLOAT;
    }

    constexpr bool type_id_is_int(TypeId type_id)
    {
        return type_id == TypeId::INT;
    }

    constexpr bool type_id_is_float(TypeId type_id)
    {
        return type_id == TypeId::FLOAT;
    }

    constexpr TypeId type_id_numeric_promote(TypeId left_type, TypeId right_type)
    {
        if(left_type == right_type) return left_type;

        return TypeId::FLOAT;
    }

    enum class NumericUnaryOperation
    {
        NEGATION,
    };

    enum class IntBinaryOperation
    {
        WF_NUMERIC_UNARY_OPERATIONS,
        MODULO,
    };

    enum class FloatBinaryOperation
    {
        WF_NUMERIC_UNARY_OPERATIONS
    };

    class Action
    {
    public:
        using polymorphic_size_enable = std::true_type;
        WF_POLYMORPHIC_SIZING

        enum class Type
        {
            STATEMENT_BLOCK,
            CREATE_STACK_VAR,
            RETURN,

            INT_UNARY,
            FLOAT_UNARY,
            INT_BINARY,
            FLOAT_BINARY,

            NUMERIC_CONVERSION,

            INT_CONSTANT,
            FLOAT_CONSTANT,

            STACK_VARIABLE_ACCESS,
        };

        Action(const SourcePosition& position, Type type)
            : type(type), position(position)
        {
        }

        virtual ~Action() = default;

        const Type type;
        const SourcePosition position;
    };

    class ExprAction : public Action
    {
    public:
        WF_POLYMORPHIC_SIZING
        ExprAction(const SourcePosition& position, Type type, TypeId result_type)
            : Action(position, type), m_result_type(result_type)
        {
        }

        TypeId get_result_type() const { return m_result_type; }
    private:
        TypeId m_result_type;
    };

    class StatementBlockAction : public Action
    {
    public:
        WF_POLYMORPHIC_SIZING

        StatementBlockAction(const SourcePosition& position, DynamicArray<Action*>&& statements,
                std::uint32_t register_count)
            : Action(position, Type::STATEMENT_BLOCK), m_statements(std::move(statements)),
                m_register_count(register_count)
        {
        }

        const DynamicArray<Action*>& get_statements() const { return m_statements; }
        std::uint32_t get_register_count() const { return m_register_count; }
    private:
        DynamicArray<Action*> m_statements;
        std::uint32_t m_register_count;
    };

    class CreateStackVariableAction : public Action
    {
    public:
        WF_POLYMORPHIC_SIZING

        CreateStackVariableAction(const SourcePosition& position, RegisterAddress address, ExprAction* initializer)
            : Action(position, Type::CREATE_STACK_VAR), m_address(address), m_initializer(initializer)
        {
        }

        RegisterAddress get_address() const { return m_address; }
        ExprAction* get_initializer() { return m_initializer; }
        const ExprAction* get_initializer() const { return m_initializer; }
    private:
        RegisterAddress m_address;
        ExprAction* m_initializer;
    };

    class ReturnAction : public Action
    {
    public:
        WF_POLYMORPHIC_SIZING

        ReturnAction(const SourcePosition& position, ExprAction* return_value)
            : Action(position, Type::RETURN), m_return_value(return_value)
        {
        }

        ExprAction* get_return_value() { return m_return_value; }
        const ExprAction* get_return_value() const { return m_return_value; }
    private:
        ExprAction* m_return_value;
    };

    template<typename OperationEnum, Action::Type ActionType>
    class UnaryAction : public ExprAction
    {
    public:
        WF_POLYMORPHIC_SIZING

        using Operation = OperationEnum;

        UnaryAction(const SourcePosition& position, Operation operation, ExprAction* operand)
            : ExprAction(position, ActionType, operand->get_result_type()), m_operation(operation), m_operand(operand)
        {
        }

        Operation get_operation() const { return m_operation; }
        ExprAction* get_operand() { return m_operand; }
        const ExprAction* get_operand() const { return m_operand; }
    private:
        Operation m_operation;
        ExprAction* m_operand;
    };

    template<typename OperationEnum, Action::Type ActionType>
    class BinaryAction : public ExprAction
    {
    public:
        WF_POLYMORPHIC_SIZING
        using Operation = OperationEnum;

        BinaryAction(const SourcePosition& position, Operation operation, TypeId result_type,
                ExprAction* left_operand, ExprAction* right_operand)
            : ExprAction(position, ActionType, result_type), m_operation(operation), m_left_operand(left_operand), m_right_operand(right_operand)
        {
        }

        Operation get_operation() const { return m_operation; }
        ExprAction* get_left_operand() { return m_left_operand; }
        const ExprAction* get_left_operand() const { return m_left_operand; }
        ExprAction* get_right_operand() { return m_right_operand; }
        const ExprAction* get_right_operand() const { return m_right_operand; }
    private:
        Operation m_operation;
        ExprAction* m_left_operand;
        ExprAction* m_right_operand;
    };

    using IntUnaryAction = UnaryAction<NumericUnaryOperation, Action::Type::INT_UNARY>;
    using FloatUnaryAction = UnaryAction<NumericUnaryOperation, Action::Type::FLOAT_UNARY>;

    using IntBinaryAction = BinaryAction<IntBinaryOperation, Action::Type::INT_BINARY>;
    using FloatBinaryAction = BinaryAction<FloatBinaryOperation, Action::Type::FLOAT_BINARY>;

    class NumericConversionAction : public ExprAction
    {
    public:
        WF_POLYMORPHIC_SIZING

        NumericConversionAction(const SourcePosition& position, TypeId result_type, ExprAction* operand)
            : ExprAction(position, Type::NUMERIC_CONVERSION, result_type), m_operand(operand)
        {
        }

        ExprAction* get_operand() { return m_operand; }
        const ExprAction* get_operand() const { return m_operand; }

        TypeId get_from_type() const { return m_operand->get_result_type(); }
    private:
        ExprAction* m_operand;
    };

    class IntConstantAction : public ExprAction
    {
    public:
        WF_POLYMORPHIC_SIZING

        IntConstantAction(const SourcePosition& position, TypeId result_type, UInt value)
            : ExprAction(position, Type::INT_CONSTANT, result_type), m_value(value)
        {
        }

        UInt get_value() const { return m_value; }

    private:
        UInt m_value;
    };

    class FloatConstantAction : public ExprAction
    {
    public:
        WF_POLYMORPHIC_SIZING

        FloatConstantAction(const SourcePosition& position, TypeId result_type, Float value)
            : ExprAction(position, Type::FLOAT_CONSTANT, result_type), m_value(value)
        {
        }

        Float get_value() const { return m_value; }
    private:
        Float m_value;
    };

    class StackVariableAccessAction : public ExprAction
    {
    public:
        WF_POLYMORPHIC_SIZING

        StackVariableAccessAction(const SourcePosition& position, TypeId result_type, RegisterAddress address)
            : ExprAction(position, Type::STACK_VARIABLE_ACCESS, result_type), m_address(address)
        {
        }

        RegisterAddress get_address() const { return m_address; }
    private:
        RegisterAddress m_address;
    };
}

#endif