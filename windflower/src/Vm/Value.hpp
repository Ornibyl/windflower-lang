#ifndef WF_VALUE_HPP
#define WF_VALUE_HPP

#include "Windflower/Windflower.hpp"

namespace wf
{
    struct Object;

    union Value
    {
        Object* as_object;
        UInt as_int;
        Float as_float;
        bool as_bool;
    };
};

#endif