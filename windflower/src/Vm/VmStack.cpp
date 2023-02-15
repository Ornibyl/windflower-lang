#include "VmStack.hpp"

namespace wf
{
    void VmStack::push(std::size_t count)
    {
        m_stack_top += count;
    }

    void VmStack::pop(std::size_t count)
    {
        m_stack_top -= count;
    }

    Value& VmStack::index(std::size_t position)
    {
        return m_stack[position];
    }

    const Value& VmStack::index(std::size_t position) const
    {
        return m_stack[position];
    }

}