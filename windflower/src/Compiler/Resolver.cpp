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
        : m_state(state), m_allocated_actions(state), m_error_manager(state), m_symbols(state)
    {
    }

    Action* Resolver::resolve_ast(Node* ast)
    {
        Action* action_tree = resolve_node(ast);

        if(m_error_manager.has_errors()) return nullptr;

        return action_tree;
    }

    std::optional<TypeId> Resolver::evaluate_type(BuiltinTypeNode* node)
    {
        return node->type_id;
    }

    ExprAction* Resolver::promote_expr(ExprAction* expr, TypeId to_type)
    {
        if(expr->get_result_type() == to_type) return expr;

        return allocate_action<NumericConversionAction>(expr->position, to_type, expr);
    }

    bool Resolver::is_implicitly_convertible_to(TypeId from, TypeId to)
    {
        return (from == to) || (from == TypeId::INT && to == TypeId::FLOAT);
    }

    Action* Resolver::resolve_node(const Node* node)
    {
        switch(node->type)
        {
            case Node::Type::STATEMENT_BLOCK: return resolve_statement_block(static_cast<const StatementBlockNode*>(node));
            case Node::Type::VARIABLE_DECLARATION: return resolve_variable_declaration(static_cast<const VariableDeclarationNode*>(node));
            case Node::Type::RETURN: return resolve_return(static_cast<const ReturnNode*>(node));

            case Node::Type::BINARY_OP:
            case Node::Type::UNARY_OP:
            case Node::Type::CONSTANT:
            case Node::Type::VARIABLE_ACCESS:
            case Node::Type::BUILTIN_TYPE:
                m_error_manager.push_error(node->position, "Resolver::resolve_node() reached an unexpected point");
                return nullptr;
        }
    }

    ExprAction* Resolver::resolve_expr(const Node* node)
    {
        switch(node->type)
        {
            case Node::Type::BINARY_OP: return resolve_binary_op(static_cast<const BinaryOpNode*>(node));
            case Node::Type::UNARY_OP: return resolve_unary_op(static_cast<const UnaryOpNode*>(node));
            case Node::Type::CONSTANT: return resolve_constant(static_cast<const ConstantNode*>(node));
            case Node::Type::VARIABLE_ACCESS: return resolve_variable_access(static_cast<const VariableAccessNode*>(node));

            case Node::Type::STATEMENT_BLOCK:
            case Node::Type::BUILTIN_TYPE:
            case Node::Type::VARIABLE_DECLARATION:
            case Node::Type::RETURN:
                m_error_manager.push_error(node->position, "Resolver::resolve_expr() reached an unexpected point");
                return nullptr;
        }
    }

    Action* Resolver::resolve_statement_block(const StatementBlockNode* node)
    {
        DynamicArray<Action*> statements(m_state);
        for(const Node* statement : node->statements)
        {
            Action* action = resolve_node(statement);
            if(action != nullptr)
            {
                statements.emplace_back(action);
            }
        }

        return allocate_action<StatementBlockAction>(node->position, std::move(statements), m_symbols.get_stack_symbol_count());
    }

    Action* Resolver::resolve_variable_declaration(const VariableDeclarationNode* node)
    {
        auto it = m_symbols.find(node->name);
        if(it != m_symbols.end())
        {
            m_error_manager.push_error(node->position,
                "'{}' was already defined when redefined here.",
                    std::string_view(node->name->text, node->name->length)
            );
            return nullptr;
        }

        ExprAction* initializer = nullptr;

        SymbolInfo& symbol_info = m_symbols.create_variable(node->name);

        if(node->storage_type != nullptr)
        {
            std::optional<TypeId> storage_type = evaluate_type(node->storage_type);
            if(!storage_type.has_value())
            {
                return nullptr;
            }
            symbol_info.storage_type = storage_type.value();
        }

        if(node->initializer != nullptr)
        {
            initializer = resolve_expr(node->initializer);
            if(initializer == nullptr) return nullptr;

            if(node->storage_type == nullptr)
            {
                symbol_info.storage_type = initializer->get_result_type();
            }
            else
            {
                if(!is_implicitly_convertible_to(initializer->get_result_type(), symbol_info.storage_type))
                {
                    m_error_manager.push_error(node->position,
                        "'{}' can not be implicitly converted to '{}'.",
                            initializer->get_result_type(),
                            symbol_info.storage_type
                    );
                    return nullptr;
                }
                initializer = promote_expr(initializer, symbol_info.storage_type);
            }
        }
        else
        {
            if(symbol_info.storage_type == TypeId::INT)
            {
                initializer = allocate_action<IntConstantAction>(node->position, TypeId::INT, 0);
            }
            else if(symbol_info.storage_type == TypeId::FLOAT)
            {
                initializer = allocate_action<FloatConstantAction>(node->position, TypeId::INT, 0.0);
            }
        }

        return allocate_action<CreateStackVariableAction>(node->position, symbol_info.address, initializer);
    }

    Action* Resolver::resolve_return(const ReturnNode* node)
    {
        if(node->return_value == nullptr)
        {
            return allocate_action<ReturnAction>(node->position, nullptr);
        }

        ExprAction* return_value = resolve_expr(node->return_value);
        return allocate_action<ReturnAction>(node->position, return_value);
    }


    ExprAction* Resolver::resolve_binary_op(const BinaryOpNode* node)
    {
        ExprAction* left_operand = resolve_expr(node->left_operand);
        ExprAction* right_operand = resolve_expr(node->right_operand);

        if(left_operand == nullptr || right_operand == nullptr) return nullptr;

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

        if(operand == nullptr) return nullptr;

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

    ExprAction* Resolver::resolve_variable_access(const VariableAccessNode* node)
    {
        auto it = m_symbols.find(node->name);

        if(it == m_symbols.end())
        {
            m_error_manager.push_error(node->position,
                "'{}' is not defined when referenced here.",
                    std::string_view(node->name->text, node->name->length)
            );
            return nullptr;
        }

        // Void for a variable indicates that its type couldn't be resolved.
        if(it->second.storage_type == TypeId::VOID) return nullptr;

        return allocate_action<StackVariableAccessAction>(node->position, it->second.storage_type, it->second.address);
    }

}