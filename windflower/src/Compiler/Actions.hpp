#ifndef WF_ACTIONS_HPP
#define WF_ACTIONS_HPP

#include "Compiler/Token.hpp"
#include "Windflower/Windflower.hpp"

namespace wf
{
    enum class TypeId
    {
        INT, FLOAT
    };

    struct Action
    {
        enum class Type
        {
            INT_UNARY,
            FLOAT_UNARY,
            INT_BINARY,
            FLOAT_BINARY,

            INT_TO_FLOAT,
            FLOAT_TO_INT,

            INT_CONSTANT,
            FLOAT_CONSTANT,
        };

        Action(Type type)
            : type(type)
        {
        }

        virtual ~Action() = default;

        const Type type;
        SourcePosition position;
    };

    struct IntUnaryAction : Action
    {
        IntUnaryAction()
            : Action(Type::INT_UNARY)
        {
        }

        enum class Operation
        {
            NEGATION,
        };

        Action* operand;
    };

    struct FloatUnaryAction : Action
    {
        FloatUnaryAction()
            : Action(Type::FLOAT_UNARY)
        {
        }

        enum class Operation
        {
            NEGATION,
        };

        Action* operand;
    };

    struct IntBinaryAction : Action
    {
        IntBinaryAction()
            : Action(Type::INT_BINARY)
        {
        }

        enum class Operation
        {
            ADD,
            SUBTRACT,
            MULTIPLY,
            DIVIDE,
            MODULO,
        };

        Action* operand;
    };

    struct FloatBinaryAction : Action
    {
        FloatBinaryAction()
            : Action(Type::FLOAT_BINARY)
        {
        }

        enum class Operation
        {
            ADD,
            SUBTRACT,
            MULTIPLY,
            DIVIDE,
        };

        Action* left_operand;
        Action* right_operand;
    };

    struct IntToFloatAction : Action
    {
        IntToFloatAction()
            : Action(Type::INT_TO_FLOAT)
        {
        }

        Action* base_action;
    };

    struct FloatToIntAction : Action
    {
        FloatToIntAction()
            : Action(Type::FLOAT_TO_INT)
        {
        }

        Action* base_action;
    };

    struct IntConstantAction : Action
    {
        IntConstantAction()
            : Action(Type::INT_CONSTANT)
        {
        }

        Int value;
    };

    struct FloatConstantAction : Action
    {
        FloatConstantAction()
            : Action(Type::FLOAT_CONSTANT)
        {
        }

        Float value;
    };
}

#endif