#ifndef WF_STATE_HPP
#define WF_STATE_HPP

#include "Windflower/Windflower.hpp"
#include "Vm/VmStack.hpp"
#include "Vm/Vm.hpp"

namespace wf
{
    struct State
    {
        State(const EnvironmentCreateInfo& create_info);
        Allocator& allocator;
        Object* allocated_objects = nullptr;

        Vm vm;
        VmStack stack;
    };
}

#endif