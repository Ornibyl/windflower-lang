#ifndef WF_NODES_HPP
#define WF_NODES_HPP

#include <string_view>

#include "Compiler/Token.hpp"
#include "Utils/Allocate.hpp"
#include "Utils/Array.hpp"
#include "Vm/Object.hpp"

namespace wf
{
    struct Node
    {
        using polymorphic_size_enable = std::true_type;
        WF_POLYMORPHIC_SIZING

        enum class Type
        {
            STATEMENT_BLOCK,
            BUILTIN_TYPE,
            VARIABLE_DECLARATION,
            RETURN,

            BINARY_OP,
            UNARY_OP,
            CONSTANT,

            VARIABLE_ACCESS,
        };

        Node(Type type)
            : type(type)
        {
        }

        virtual ~Node() = default;

        const Type type;
        SourcePosition position;
    };

    struct StatementBlockNode : Node
    {
        WF_POLYMORPHIC_SIZING

        StatementBlockNode(State* state)
            : Node(Type::STATEMENT_BLOCK), statements(state)
        {
        }

        DynamicArray<Node*> statements;
    };

    struct BuiltinTypeNode : Node
    {
        WF_POLYMORPHIC_SIZING

        BuiltinTypeNode()
            : Node(Type::STATEMENT_BLOCK)
        {
        }

        TypeId type_id;
    };

    struct VariableDeclarationNode : Node
    {
        WF_POLYMORPHIC_SIZING

        VariableDeclarationNode()
            : Node(Type::VARIABLE_DECLARATION)
        {
        }

        StringObject* name;
        Node* initializer;
        BuiltinTypeNode* storage_type;
    };

    struct ReturnNode : Node
    {
        WF_POLYMORPHIC_SIZING

        ReturnNode()
            : Node(Type::RETURN)
        {
        }

        Node* return_value;
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

    struct VariableAccessNode : Node
    {
        WF_POLYMORPHIC_SIZING

        VariableAccessNode()
            : Node(Type::VARIABLE_ACCESS)
        {
        }

        StringObject* name;
    };
}

#endif