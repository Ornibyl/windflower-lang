#ifndef WF_VALUE_HPP
#define WF_VALUE_HPP

#include "Windflower/Windflower.hpp"

namespace wf
{
    struct Object;
    struct BytecodeObject;

    union Value
    {
        Object* as_object;
        UInt as_int;
        Float as_float;
        bool as_bool;

        BytecodeObject* as_bytecode() const;
    };
};

#endif