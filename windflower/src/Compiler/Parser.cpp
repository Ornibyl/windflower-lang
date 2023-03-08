#include "Parser.hpp"

#include "Utils/Numeric.hpp"

namespace wf
{
    Parser::Parser(State* state, const CompileInfo& compile_info)
        : m_state(state), m_tokenizer(compile_info.name, compile_info.source), m_error_manager(state),
            m_newline_ignore_stack(state), m_allocated_nodes(state)
    {
        advance();
    }

    Node* Parser::parse()
    {
        push_newline_ignore(false);

        StatementBlockNode* ast = parse_statement_block();

        pop_newline_ignore();

        if(!m_error_manager.has_errors() && m_current.get_type() != Token::Type::TT_EOF)
        {
            push_error(m_current.get_position(), "Expected a newline.");
        }

        if(m_error_manager.has_errors()) return nullptr;

        return ast;
    }

    void Parser::push_expected_expr_error(const SourcePosition& position)
    {
        push_error(position, "Expected an expression.");
    }

    void Parser::push_expected_identifier_error(const SourcePosition& position)
    {
        push_error(position, "Expected an identifier.");
    }

    void Parser::push_expected_storage_type_error(const SourcePosition& position)
    {
        push_error(position, "Expected a storage type.");
    }

    void Parser::advance()
    {
        m_current = m_tokenizer.next();

        while(m_current.get_type() == Token::Type::ERROR)
        {
            push_error(m_current.get_position(), "{}", m_current.get_text());
            m_current = m_tokenizer.next();
        }
    }

    void Parser::push_newline_ignore(bool value)
    {
        m_newline_ignore_stack.push(value);
        m_tokenizer.set_newline_ignore(m_newline_ignore_stack.top());
    }

    void Parser::pop_newline_ignore()
    {
        m_newline_ignore_stack.pop();
        m_tokenizer.set_newline_ignore(m_newline_ignore_stack.empty()? false : m_newline_ignore_stack.top());
    }

    BuiltinTypeNode* Parser::parse_builtin_type()
    {
        BuiltinTypeNode* node = allocate_node<BuiltinTypeNode>();
        node->position = m_current.get_position();
        switch(m_current.get_type())
        {
            case Token::Type::KW_INT:
                node->type_id = TypeId::INT;
                break;
            case Token::Type::KW_FLOAT:
                node->type_id = TypeId::FLOAT;
                break;
            default:
                return nullptr;
        }
        advance();
        return node;
    }

    StatementBlockNode* Parser::parse_statement_block()
    {
        while(m_current.get_type() == Token::Type::NEWLINE) advance();

        StatementBlockNode* node = allocate_node<StatementBlockNode>(m_state);
        node->position = m_current.get_position();

        while(m_current.get_type() != Token::Type::TT_EOF)
        {
            Node* statement = parse_statement();
            if(statement == nullptr)
            {
                while(m_current.get_type() != Token::Type::NEWLINE && m_current.get_type() != Token::Type::TT_EOF)
                {
                    advance();
                }
            }
            else
            {
                node->statements.emplace_back(statement);
            }

            std::uint32_t newline_count = 0;
            while(m_current.get_type() == Token::Type::NEWLINE)
            {
                newline_count++;
                advance();
            }

            if(newline_count == 0) break;
        }
        return node;
    }

    Node* Parser::parse_statement()
    {
        switch(m_current.get_type())
        {
            case Token::Type::KW_VAR: return parse_variable_declaration();
            case Token::Type::KW_RETURN: return parse_return();
            default:
                break;
        }

        SourcePosition expr_position = m_current.get_position();
        Node* expr = parse_expression();
        if(expr == nullptr)
        {
           push_expected_expr_error(expr_position);
        }
        return expr;
    }

    VariableDeclarationNode* Parser::parse_variable_declaration()
    {
        VariableDeclarationNode* node = allocate_node<VariableDeclarationNode>();
        node->position = m_current.get_position();
        advance();

        if(m_current.get_type() != Token::Type::IDENTIFIER)
        {
            push_expected_identifier_error(m_current.get_position());
            return nullptr;
        }
        node->name = StringObject::from_text(m_state, m_current.get_text());
        advance();

        if(m_current.get_type() != Token::Type::COLON_EQUALS)
        {
            if(m_current.get_type() != Token::Type::COLON)
            {
                push_error(m_current.get_position(), "A variable declaration must contain an explicit type annotation or an initial value.");
                return nullptr;
            }
            advance();

            SourcePosition storage_type_position = m_current.get_position();
            node->storage_type = parse_builtin_type();
            if(node->storage_type == nullptr)
            {
                push_expected_storage_type_error(storage_type_position);
            }

            if(m_current.get_type() != Token::Type::COLON_EQUALS)
            {
                return node;
            }
        }

        advance();

        SourcePosition expr_position = m_current.get_position();
        node->initializer = parse_expression();
        if(node->initializer == nullptr)
        {
            push_expected_expr_error(expr_position);
            return nullptr;
        }
        return node;
    }

    ParameterNode* Parser::parse_parameter()
    {
        ParameterNode* node = allocate_node<ParameterNode>();
        node->position = m_current.get_position();

        if(m_current.is_keyword())
        {
            node->argument_label = StringObject::from_text(m_state, m_current.get_text());
            advance();
            if(m_current.get_type() == Token::Type::COLON)
            {
                push_error(node->position, "An keyword can only be used as an argument label if the parameter name is specified seperately.");
                return nullptr;
            }
            else if(m_current.get_type() != Token::Type::IDENTIFIER)
            {
                push_expected_identifier_error(m_current.get_position());
                return nullptr;
            }

            node->name = StringObject::from_text(m_state, m_current.get_text());
            advance();
        }
        else
        {
            if(m_current.get_type() != Token::Type::IDENTIFIER)
            {
                push_expected_identifier_error(m_current.get_position());
                return nullptr;
            }
            node->argument_label = StringObject::from_text(m_state, m_current.get_text());
            node->name = node->argument_label;
            advance();

            if(m_current.get_type() == Token::Type::IDENTIFIER)
            {
                node->name = StringObject::from_text(m_state, m_current.get_text());
                advance();
            }
        }

        if(m_current.get_type() != Token::Type::COLON)
        {
            push_error(m_current.get_position(), "Expected a ':'.");
        }
        advance();

        SourcePosition storage_type_position = m_current.get_position();
        node->storage_type = parse_builtin_type();
        if(node->storage_type == nullptr)
        {
            push_expected_storage_type_error(storage_type_position);
        }

        return node;
    }

    ArgumentNode* Parser::parse_argument()
    {
        ArgumentNode* node = allocate_node<ArgumentNode>();
        node->position = m_current.get_position();
        node->label = nullptr;

        if(m_current.is_keyword())
        {
            node->label = StringObject::from_text(m_state, m_current.get_text());
            advance();
            if(m_current.get_type() != Token::Type::COLON)
            {
                push_error(m_current.get_position(), "Expected a ':'.");
                return nullptr;
            }
            advance();
        }

        SourcePosition expr_position = m_current.get_position();
        node->value = parse_expression();
        if(node->value == nullptr)
        {
            push_expected_expr_error(expr_position);
            return nullptr;
        }
        if(node->label != nullptr && m_current.get_type() == Token::Type::COLON
            && node->value->type == Node::Type::VARIABLE_ACCESS)
        {
            node->label = static_cast<VariableAccessNode*>(node->value)->name;
            advance();

            expr_position = m_current.get_position();
            node->value = parse_expression();
            if(node->value == nullptr)
            {
                push_expected_expr_error(expr_position);
                return nullptr;
            }
        }
        return node;
    }

    ExternFunctionDeclarationNode* Parser::parse_extern_function_declaration()
    {
        ExternFunctionDeclarationNode* node = allocate_node<ExternFunctionDeclarationNode>(m_state);
        node->position = m_current.get_position();
        advance();

        if(m_current.get_type() != Token::Type::IDENTIFIER)
        {
            push_expected_identifier_error(m_current.get_position());
            return nullptr;
        }
        node->name = StringObject::from_text(m_state, m_current.get_text());
        advance();

        if(m_current.get_type() != Token::Type::LEFT_PAREN)
        {
            push_error(m_current.get_position(), "Expected a '('.");
            return nullptr;
        }
        SourcePosition paren_position = m_current.get_position();
        advance();

        if(m_current.get_type() != Token::Type::RIGHT_PAREN)
        {
            ParameterNode* parameter = parse_parameter();
            if(parameter == nullptr) return nullptr;
            node->parameters.emplace_back(parameter);

            while(m_current.get_type() == Token::Type::COMMA)
            {
                advance();
                ParameterNode* parameter = parse_parameter();
                if(parameter == nullptr) return nullptr;
                node->parameters.emplace_back(parameter);
            }

            if(m_current.get_type() != Token::Type::RIGHT_PAREN)
            {
                push_error(m_current.get_position(), "Expected a ')' to match the '( at {}", paren_position);
            }
        }
        advance();

        if(m_current.get_type() != Token::Type::ARROW)
        {
            push_error(m_current.get_position(), "Expected an '->'.");
            return nullptr;
        }
        advance();

        SourcePosition storage_type_position = m_current.get_position();
        node->return_type = parse_builtin_type();
        if(node->return_type == nullptr)
        {
            push_expected_storage_type_error(storage_type_position);
        }

        return node;
    }

    ReturnNode* Parser::parse_return()
    {
        ReturnNode* node = allocate_node<ReturnNode>();
        node->position = m_current.get_position();
        advance();

        if(m_current.get_type() == Token::Type::NEWLINE)
        {
            node->return_value = nullptr;
            return node;
        }

        SourcePosition expr_position = m_current.get_position();
        node->return_value = parse_expression();
        if(node->return_value == nullptr)
        {
            push_expected_expr_error(expr_position);
            return nullptr;
        }
        return node;
    }

    Node* Parser::parse_expression(ExprPrecedence precedence)
    {
        ExprPrefixFunc prefix = get_rule(m_current.get_type()).prefix;
        if(prefix == nullptr)
        {
            return nullptr;
        }

        Node* prev = (this->*prefix)();
        if(prev == nullptr)
        {
            return nullptr;
        }

        if(get_rule(m_current.get_type()).infix == nullptr)
        {
            return prev;
        }

        while(precedence <= get_rule(m_current.get_type()).precedence)
        {
            ExprInfixFunc infix_func = get_rule(m_current.get_type()).infix;
            prev = (this->*infix_func)(prev);

            if(prev == nullptr)
            {
                return nullptr;
            }
        }

        return prev;
    }

    Node* Parser::parse_binary_op(Node* prev)
    {
        Token op_token = m_current;
        advance();

        BinaryOpNode* node = allocate_node<BinaryOpNode>();
        node->position = op_token.get_position();
        node->left_operand = prev;

        ExprPrecedence precedence = static_cast<ExprPrecedence>(
            to_underlying(get_rule(op_token.get_type()).precedence) + 1
        );

        SourcePosition right_operand_position = m_current.get_position();
        node->right_operand = parse_expression(precedence);

        if(node->right_operand == nullptr)
        {
            push_expected_expr_error(right_operand_position);
            return nullptr;
        }

        switch(op_token.get_type())
        {
            case Token::Type::PLUS:
                node->operation = BinaryOpNode::Operation::ADD;
                break;
            case Token::Type::MINUS:
                node->operation = BinaryOpNode::Operation::SUBTRACT;
                break;
            case Token::Type::STAR:
                node->operation = BinaryOpNode::Operation::MULTIPLY;
                break;
            case Token::Type::SLASH:
                node->operation = BinaryOpNode::Operation::DIVIDE;
                break;
            case Token::Type::PERCENT:
                node->operation = BinaryOpNode::Operation::MODULO;
                break;
            default:
                push_error(m_current.get_position(), "Parser::parse_binary_op() reached an unexpected point.");
                return nullptr;
        }
        return node;
    }

    Node* Parser::parse_unary_op()
    {
        UnaryOpNode* node = allocate_node<UnaryOpNode>();
        node->position = m_current.get_position();
        switch(m_current.get_type())
        {
            case Token::Type::PLUS:
                node->operation = UnaryOpNode::Operation::PLUS;
                break;
            case Token::Type::MINUS:
                node->operation = UnaryOpNode::Operation::NEGATE;
                break;
            default:
                push_error(m_current.get_position(), "Parser::parse_unary_op() reached an unexpected point.");
                return nullptr;
        }
        advance();

        SourcePosition operand_position = m_current.get_position();
        node->operand = parse_expression(ExprPrecedence::SIGN);
        if(node->operand == nullptr)
        {
            push_expected_expr_error(operand_position);
            return nullptr;
        }

        return node;
    }

    Node* Parser::parse_constant()
    {
        ConstantNode* node = allocate_node<ConstantNode>();
        node->position = m_current.get_position();
        node->value = m_current.get_text();
        switch(m_current.get_type())
        {
            case Token::Type::INT_CONSTANT:
                node->value_type = ConstantNode::ValueType::INT;
                break;
            case Token::Type::FLOAT_CONSTANT:
                node->value_type = ConstantNode::ValueType::FLOAT;
                break;
            default:
                push_error(m_current.get_position(), "Parser::parse_constant() reached an unexpected point.");
                return nullptr;
        }

        advance();

        return node;
    }

    Node* Parser::parse_variable_access()
    {
        VariableAccessNode* node = allocate_node<VariableAccessNode>();
        node->position = m_current.get_position();
        node->name = StringObject::from_text(m_state, m_current.get_text());
        advance();
        return node;
    }

    Node* Parser::parse_grouping()
    {
        SourcePosition paren_position = m_current.get_position();
        advance();
        SourcePosition expr_position = m_current.get_position();

        Node* expr = parse_expression();

        if(expr == nullptr)
        {
            push_expected_expr_error(expr_position);
            return nullptr;
        }

        if(m_current.get_type() != Token::Type::RIGHT_PAREN)
        {
            push_error(m_current.get_position(), "Expected a ')' to match the '( at {}", paren_position);
        }

        advance();

        return expr;
    }

    Node* Parser::parse_call(Node* prev)
    {
        CallNode* node = allocate_node<CallNode>(m_state);
        node->position = m_current.get_position();
        advance();

        if(m_current.get_type() != Token::Type::RIGHT_PAREN)
        {
            ArgumentNode* argument = parse_argument();
            if(argument == nullptr) return nullptr;
            node->arguments.emplace_back(argument);

            while(m_current.get_type() == Token::Type::COMMA)
            {
                advance();
                argument = parse_argument();
                if(argument == nullptr) return nullptr;
                node->arguments.emplace_back(argument);
            }

            if(m_current.get_type() != Token::Type::RIGHT_PAREN)
            {
                push_error(m_current.get_position(), "Expected a ')' to match the '( at {}", node->position);
            }
        }

        advance();

        return node;
    }

    const Parser::ExprRule& Parser::get_rule(Token::Type type)
    {
        static const auto rules = arr_from_designators<ExprRule, static_cast<std::size_t>(Token::Type::TT_COUNT), Token::Type>({
            { Token::Type::TT_EOF,          {   nullptr,                            nullptr,                        ExprPrecedence::NONE                } },
            { Token::Type::ERROR,           {   nullptr,                            nullptr,                        ExprPrecedence::NONE                } },

            { Token::Type::INT_CONSTANT,    {   &Parser::parse_constant,            nullptr,                        ExprPrecedence::NONE                } },
            { Token::Type::FLOAT_CONSTANT,  {   &Parser::parse_constant,            nullptr,                        ExprPrecedence::NONE                } },

            { Token::Type::IDENTIFIER,      {   &Parser::parse_variable_access,     nullptr,                        ExprPrecedence::NONE                } },

            { Token::Type::PLUS,            {   &Parser::parse_unary_op,            &Parser::parse_binary_op,       ExprPrecedence::ADDITIVE            } },
            { Token::Type::MINUS,           {   &Parser::parse_unary_op,            &Parser::parse_binary_op,       ExprPrecedence::ADDITIVE            } },
            { Token::Type::STAR,            {   nullptr,                            &Parser::parse_binary_op,       ExprPrecedence::MULTIPLICATIVE      } },
            { Token::Type::SLASH,           {   nullptr,                            &Parser::parse_binary_op,       ExprPrecedence::MULTIPLICATIVE      } },
            { Token::Type::PERCENT,         {   nullptr,                            &Parser::parse_binary_op,       ExprPrecedence::MULTIPLICATIVE      } },

            { Token::Type::LEFT_PAREN,      {   &Parser::parse_grouping,            &Parser::parse_call,            ExprPrecedence::CALL                } },
            { Token::Type::RIGHT_PAREN,     {   nullptr,                            nullptr,                        ExprPrecedence::NONE                } },
        });

        return rules[static_cast<std::size_t>(type)];
    }
}