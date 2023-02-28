#ifndef WF_RESOLVER_HPP
#define WF_RESOLVER_HPP

#include <concepts>

#include "Compiler/ErrorManager.hpp"
#include "Utils/Array.hpp"
#include "Utils/ScopedPtr.hpp"

#include "Nodes.hpp"
#include "Actions.hpp"

namespace wf
{
    class Resolver
    {
    public:
        Resolver(State* state);

        Action* resolve_ast(Node* ast);
        const String& get_error_message () const { return m_error_manager.get_message(); }
    private:
        State* const m_state;
        DynamicArray<ScopedPtr<Action>> m_allocated_actions;
        ErrorManager m_error_manager;

        ExprAction* promote_expr(ExprAction* expr, TypeId to_type);

        Action* resolve_node(const Node* node);
        ExprAction* resolve_expr(const Node* node);
        ExprAction* resolve_binary_op(const BinaryOpNode* node);
        ExprAction* resolve_unary_op(const UnaryOpNode* node);
        ExprAction* resolve_constant(const ConstantNode* node);

        template<typename T, typename... Args> requires(std::derived_from<T, Action> && std::constructible_from<T, Args...>)
        T* allocate_action(Args&&... args)
        {
            m_allocated_actions.push_back(construct_scoped<T>(m_state, std::forward<Args>(args)...));
            return static_cast<T*>(m_allocated_actions.back().get());
        }
    };
}

#endif