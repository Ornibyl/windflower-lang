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
        push_newline_ignore(true);

        SourcePosition expr_position = m_current.get_position();
        Node* ast = parse_expression();
        if(ast == nullptr)
        {
            m_error_manager.push_error(expr_position, "Expected an expression");
        }

        if(!m_error_manager.has_errors() && m_current.get_type() != Token::Type::TT_EOF)
        {
            m_error_manager.push_error(m_current.get_position(), "Expected an operator.");
        }

        pop_newline_ignore();
        return ast;
    }

    void Parser::advance()
    {
        m_current = m_tokenizer.next();

        while(m_current.get_type() == Token::Type::ERROR)
        {
            m_error_manager.push_error(m_current.get_position(), "{}", m_current.get_text());
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
            m_error_manager.push_error(right_operand_position, "Expected an expression.");
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
                throw std::runtime_error("Parser::parse_binary_op() reached an unexpected point.");
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
                m_error_manager.push_error(m_current.get_position(), "Parser::parse_unary_op() reached an unexpected point.");
                return nullptr;
        }
        advance();

        SourcePosition operand_position = m_current.get_position();
        node->operand = parse_expression(ExprPrecedence::SIGN);
        if(node->operand == nullptr)
        {
            m_error_manager.push_error(operand_position, "Expected an expression.");
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
                m_error_manager.push_error(m_current.get_position(), "Parser::parse_constant() reached an unexpected point.");
                return nullptr;
        }

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
            m_error_manager.push_error(expr_position, "Expected an expression");
            return nullptr;
        }

        if(m_current.get_type() != Token::Type::RIGHT_PAREN)
        {
            m_error_manager.push_error(expr_position, "Expected a ')' to match the '( at {}", paren_position);
        }

        advance();

        return expr;
    }

    const Parser::ExprRule& Parser::get_rule(Token::Type type)
    {
        static const auto rules = arr_from_designators<ExprRule, static_cast<std::size_t>(Token::Type::TT_COUNT), Token::Type>({
            { Token::Type::TT_EOF,          {   nullptr,                    nullptr,                       ExprPrecedence::NONE               } },
            { Token::Type::ERROR,           {   nullptr,                    nullptr,                       ExprPrecedence::NONE               } },

            { Token::Type::INT_CONSTANT,    {   &Parser::parse_constant,    nullptr,                       ExprPrecedence::NONE               } },
            { Token::Type::FLOAT_CONSTANT,  {   &Parser::parse_constant,    nullptr,                       ExprPrecedence::NONE               } },

            { Token::Type::PLUS,            {   &Parser::parse_unary_op,    &Parser::parse_binary_op,      ExprPrecedence::ADDITIVE           } },
            { Token::Type::MINUS,           {   &Parser::parse_unary_op,    &Parser::parse_binary_op,      ExprPrecedence::ADDITIVE           } },
            { Token::Type::STAR,            {   nullptr,                    &Parser::parse_binary_op,      ExprPrecedence::MULTIPLICATIVE     } },
            { Token::Type::SLASH,           {   nullptr,                    &Parser::parse_binary_op,      ExprPrecedence::MULTIPLICATIVE     } },
            { Token::Type::PERCENT,         {   nullptr,                    &Parser::parse_binary_op,      ExprPrecedence::MULTIPLICATIVE     } },

            { Token::Type::LEFT_PAREN,      {   &Parser::parse_grouping,    nullptr,                       ExprPrecedence::NONE               } },
            { Token::Type::RIGHT_PAREN,     {   nullptr,                    nullptr,                       ExprPrecedence::NONE               } },
        });

        return rules[static_cast<std::size_t>(type)];
    }
}