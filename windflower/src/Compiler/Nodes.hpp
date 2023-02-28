#ifndef WF_NODES_HPP
#define WF_NODES_HPP

#include <string_view>

#include "Compiler/Token.hpp"
#include "Utils/Allocate.hpp"

namespace wf
{
    struct Node
    {
        using polymorphic_size_enable = std::true_type;
        WF_POLYMORPHIC_SIZING

        enum class Type
        {
            BINARY_OP,
            UNARY_OP,
            CONSTANT,
        };

        Node(Type type)
            : type(type)
        {
        }

        virtual ~Node() = default;

        const Type type;
        SourcePosition position;
    };

    struct BinaryOpNode : Node
    {
        WF_POLYMORPHIC_SIZING

        BinaryOpNode()
            : Node(Type::BINARY_OP)
        {
        }

        enum class Operation
        {
            ADD, SUBTRACT, MULTIPLY, DIVIDE, MODULO
        };

        Node* left_operand;
        Node* right_operand;
        Operation operation;
    };

    struct UnaryOpNode : Node
    {
        WF_POLYMORPHIC_SIZING

        UnaryOpNode()
            : Node(Type::UNARY_OP)
        {
        }

        enum class Operation
        {
            PLUS, NEGATE
        };

        Node* operand;
        Operation operation;
    };

    struct ConstantNode : Node
    {
        WF_POLYMORPHIC_SIZING

        ConstantNode()
            : Node(Type::CONSTANT)
        {
        }

        enum class ValueType
        {
            INT, FLOAT
        };

        ValueType value_type;
        std::string_view value;
    };
}

#endif