#include "Object.hpp"

#include "State.hpp"

namespace wf
{
    BytecodeObject* Value::as_bytecode() const
    {
        return static_cast<BytecodeObject*>(as_object);
    }

    Object::Object(State* state)
    {
        next = state->allocated_objects;
        state->allocated_objects = this;
    }
}