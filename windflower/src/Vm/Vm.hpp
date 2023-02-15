#ifndef WF_VM_HPP
#define WF_VM_HPP

#include "VmStack.hpp"

namespace wf
{
    class Vm
    {
    public:
        Vm(State* state) : m_state(state) { (void)m_state; };
    private:
        State* m_state;
    };
}

#endif