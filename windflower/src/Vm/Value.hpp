#ifndef WF_VALUE_HPP
#define WF_VALUE_HPP

#include "Windflower/Windflower.hpp"

namespace wf
{
    struct Object;
    struct BytecodeObject;
    struct StringObject;

    union Value
    {
        Value() = default;

        Value(Object* value)
            : as_object(value)
        {
        }

        Value(UInt value)
            : as_int(value)
        {
        }

        Value(Float value)
            : as_float(value)
        {
        }

        Value(bool value)
            : as_bool(value)
        {
        }

        Object* as_object;
        UInt as_int;
        Float as_float;
        bool as_bool;

        BytecodeObject* as_bytecode() const;
        StringObject* as_string() const;
    };
};

#endif