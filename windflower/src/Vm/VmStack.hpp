#ifndef WF_VM_STACK_HPP
#define WF_VM_STACK_HPP

#include <array>
#include <cstddef>

#include "Value.hpp"

namespace wf
{
    class VmStack
    {
    public:
        static constexpr std::size_t MAX_STACK_SIZE = 1024 * 256;

        void push(std::size_t count);
        void pop(std::size_t count);

        Value& index(std::size_t position);
        const Value& index(std::size_t position) const;

        decltype(auto) begin() { return m_stack.begin(); }
        decltype(auto) begin() const { return m_stack.begin(); }
        decltype(auto) cbegin() const { return m_stack.cbegin(); }

        decltype(auto) rbegin() { return m_stack.rbegin(); }
        decltype(auto) rbegin() const { return m_stack.rbegin(); }
        decltype(auto) crbegin() const { return m_stack.crbegin(); }

        decltype(auto) end() { return m_stack.cend(); }
        decltype(auto) end() const { return m_stack.cend(); }
        decltype(auto) cend() const { return m_stack.cend(); }

        decltype(auto) rend() { return m_stack.rend(); }
        decltype(auto) rend() const { return m_stack.rend(); }
        decltype(auto) crend() const { return m_stack.crend(); }
    private:
        std::array<Value, MAX_STACK_SIZE> m_stack;
        std::size_t m_stack_top = 0;
    };
}

#endif