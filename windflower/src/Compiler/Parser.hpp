#ifndef WF_PARSER_HPP
#define WF_PARSER_HPP

#include <stack>
#include <concepts>

#include "Nodes.hpp"
#include "Tokenizer.hpp"

#include "Utils/Array.hpp"
#include "Utils/ScopedPtr.hpp"
#include "ErrorManager.hpp"

namespace wf
{
    class Parser
    {
    public:
        Parser(State* state, const CompileInfo& compile_info);

        Node* parse();
        const String& get_error_message() const { return m_error_manager.get_message(); }
    private:
        State* const m_state;
        Tokenizer m_tokenizer;
        ErrorManager m_error_manager;
        std::stack<bool, DynamicArray<bool>> m_newline_ignore_stack;

        DynamicArray<ScopedPtr<Node>> m_allocated_nodes;

        Token m_current;

        enum class ExprPrecedence
        {
            NONE,
            ADDITIVE,
            MULTIPLICATIVE,
            SIGN,
            PRIMARY,
        };

        using ExprPrefixFunc = Node*(Parser::*)();
        using ExprInfixFunc = Node*(Parser::*)(Node* prev);

        struct ExprRule
        {
            ExprPrefixFunc prefix;
            ExprInfixFunc infix;
            ExprPrecedence precedence;
        };

        void advance();

        void push_newline_ignore(bool value);
        void pop_newline_ignore();

        Node* parse_expression(ExprPrecedence precedence = ExprPrecedence::ADDITIVE);
        Node* parse_binary_op(Node* prev);
        Node* parse_unary_op();
        Node* parse_constant();
        Node* parse_grouping();

        static const ExprRule& get_rule(Token::Type type);

        template<typename T, typename... Args> requires(std::derived_from<T, Node> && std::constructible_from<T, Args...>)
        T* allocate_node(Args&&... args)
        {
            m_allocated_nodes.push_back(construct_scoped<T>(m_state, std::forward<Args>(args)...));
            return static_cast<T*>(m_allocated_nodes.back().get());
        }
    };
}

#endif