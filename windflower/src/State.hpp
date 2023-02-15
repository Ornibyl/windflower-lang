#ifndef WF_STATE_HPP
#define WF_STATE_HPP

#include "Windflower/Windflower.hpp"
#include "Vm/VmStack.hpp"

namespace wf
{
    struct State
    {
        State(const EnvironmentCreateInfo& create_info);
        Allocator& allocator;
        VmStack stack;
    };
}

#endif