#include "Resolver.hpp"

template<>
struct fmt::formatter<wf::TypeId>
{
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const wf::TypeId& type_id, FormatContext& ctx)
    {
        return fmt::format_to(ctx.out(), "{}", type_id_to_string(type_id));
    }
};

template<>
struct fmt::formatter<wf::BinaryOpNode::Operation>
{
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const wf::BinaryOpNode::Operation& operation, FormatContext& ctx)
    {
        std::string_view result;
        switch(operation)
        {
            case wf::BinaryOpNode::Operation::ADD:
                result = "+";
                break;
            case wf::BinaryOpNode::Operation::SUBTRACT:
                result = "-";
                break;
            case wf::BinaryOpNode::Operation::MULTIPLY:
                result = "*";
                break;
            case wf::BinaryOpNode::Operation::DIVIDE:
                result = "/";
                break;
            case wf::BinaryOpNode::Operation::MODULO:
                result = "%";
                break;
        }
        return fmt::format_to(ctx.out(), "{}", result);
    }
};

template<>
struct fmt::formatter<wf::UnaryOpNode::Operation>
{
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const wf::UnaryOpNode::Operation& operation, FormatContext& ctx)
    {
        std::string_view result;
        switch(operation)
        {
            case wf::UnaryOpNode::Operation::PLUS:
                return "+";
            case wf::UnaryOpNode::Operation::NEGATE:
                return "-";
        }
        return fmt::format_to(ctx.out(), "{}", result);
    }
};

namespace wf
{
    Resolver::Resolver(State* state)
        : m_state(state), m_allocated_actions(state), m_error_manager(state)
    {
    }

    Action* Resolver::resolve_ast(Node* ast)
    {
        return resolve_node(ast);
    }

    ExprAction* Resolver::promote_expr(ExprAction* expr, TypeId to_type)
    {
        if(expr->get_result_type() == to_type) return expr;

        return allocate_action<NumericConversion>(expr->position, to_type, expr);
    }

    Action* Resolver::resolve_node(const Node* node)
    {
        switch(node->type)
        {
            case Node::Type::BINARY_OP: return resolve_binary_op(static_cast<const BinaryOpNode*>(node));
            case Node::Type::UNARY_OP: return resolve_unary_op(static_cast<const UnaryOpNode*>(node));
            case Node::Type::CONSTANT: return resolve_constant(static_cast<const ConstantNode*>(node));
        }
    }

    ExprAction* Resolver::resolve_expr(const Node* node)
    {
        switch(node->type)
        {
            case Node::Type::BINARY_OP: return resolve_binary_op(static_cast<const BinaryOpNode*>(node));
            case Node::Type::UNARY_OP: return resolve_unary_op(static_cast<const UnaryOpNode*>(node));
            case Node::Type::CONSTANT: return resolve_constant(static_cast<const ConstantNode*>(node));
        }
    }

    ExprAction* Resolver::resolve_binary_op(const BinaryOpNode* node)
    {
        ExprAction* left_operand = resolve_expr(node->left_operand);
        ExprAction* right_operand = resolve_expr(node->right_operand);

        switch(node->operation)
        {
            case BinaryOpNode::Operation::ADD:
            case BinaryOpNode::Operation::SUBTRACT:
            case BinaryOpNode::Operation::MULTIPLY:
            case BinaryOpNode::Operation::DIVIDE:
            {
                if(!type_id_is_numeric(left_operand->get_result_type()) || !type_id_is_numeric(right_operand->get_result_type()))
                {
                    m_error_manager.push_error(node->position,
                        "Cannot perform '{}' with operands of type '{}' and '{}",
                            node->operation,
                            left_operand->get_result_type(),
                            right_operand->get_result_type()
                    );
                    return nullptr;
                }

                TypeId result_type = type_id_numeric_promote(left_operand->get_result_type(), right_operand->get_result_type());
                left_operand = promote_expr(left_operand, result_type);
                right_operand = promote_expr(right_operand, result_type);

                if(type_id_is_int(result_type))
                {
                    IntBinaryOperation operation;
                    switch(node->operation)
                    {
                        case BinaryOpNode::Operation::ADD:
                            operation = IntBinaryOperation::ADD;
                            break;
                        case BinaryOpNode::Operation::SUBTRACT:
                            operation = IntBinaryOperation::SUBTRACT;
                            break;
                        case BinaryOpNode::Operation::MULTIPLY:
                            operation = IntBinaryOperation::MULTIPLY;
                            break;
                        case BinaryOpNode::Operation::DIVIDE:
                            operation = IntBinaryOperation::DIVIDE;
                            break;
                        default:
                            return nullptr;
                    }
                    return allocate_action<IntBinaryAction>(node->position, operation, result_type,
                            left_operand, right_operand);
                }
                else // is float
                {
                    FloatBinaryOperation operation;
                    switch(node->operation)
                    {
                        case BinaryOpNode::Operation::ADD:
                            operation = FloatBinaryOperation::ADD;
                            break;
                        case BinaryOpNode::Operation::SUBTRACT:
                            operation = FloatBinaryOperation::SUBTRACT;
                            break;
                        case BinaryOpNode::Operation::MULTIPLY:
                            operation = FloatBinaryOperation::MULTIPLY;
                            break;
                        case BinaryOpNode::Operation::DIVIDE:
                            operation = FloatBinaryOperation::DIVIDE;
                            break;
                        default:
                            return nullptr;
                    }
                    return allocate_action<FloatBinaryAction>(node->position, operation, result_type,
                            left_operand, right_operand);
                }
                break;
            }
            case BinaryOpNode::Operation::MODULO:
            {
                if(!type_id_is_int(left_operand->get_result_type()) || !type_id_is_int(right_operand->get_result_type()))
                {
                    m_error_manager.push_error(node->position,
                        "Cannot perform '{}' with operands of type '{}' and '{}",
                            node->operation,
                            left_operand->get_result_type(),
                            right_operand->get_result_type()
                    );
                    return nullptr;
                }

                TypeId result_type = type_id_numeric_promote(left_operand->get_result_type(), right_operand->get_result_type());
                left_operand = promote_expr(left_operand, result_type);
                right_operand = promote_expr(right_operand, result_type);

                return allocate_action<IntBinaryAction>(node->position, IntBinaryOperation::MODULO, result_type,
                        left_operand, right_operand);
            }
        }
    }

    ExprAction* Resolver::resolve_unary_op(const UnaryOpNode* node)
    {
        ExprAction* operand = resolve_expr(node->operand);
        switch(node->operation)
        {
            case UnaryOpNode::Operation::PLUS: return operand;
            case UnaryOpNode::Operation::NEGATE:
                switch(operand->get_result_type())
                {
                    case TypeId::INT:
                        return allocate_action<IntUnaryAction>(node->position,
                                NumericUnaryOperation::NEGATION, operand);
                    case TypeId::FLOAT:
                        return allocate_action<FloatUnaryAction>(node->position,
                                NumericUnaryOperation::NEGATION, operand);
                    default:
                        return nullptr;
                }
        }
    }

    ExprAction* Resolver::resolve_constant(const ConstantNode* node)
    {
        switch(node->value_type)
        {
            case ConstantNode::ValueType::INT:
                return allocate_action<IntConstantAction>(node->position, TypeId::INT,
                        std::strtoull(node->value.data(), nullptr, 10));
            case ConstantNode::ValueType::FLOAT:
                return allocate_action<FloatConstantAction>(node->position, TypeId::FLOAT,
                        std::strtod(node->value.data(), nullptr));
        }
    }

}