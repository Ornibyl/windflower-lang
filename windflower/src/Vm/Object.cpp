#include "Object.hpp"

#include "State.hpp"

namespace wf
{
    BytecodeObject* Value::as_bytecode() const
    {
        return static_cast<BytecodeObject*>(as_object);
    }

    StringObject* Value::as_string() const
    {
        return static_cast<StringObject*>(as_object);
    }

    Object::Object(State* state)
    {
        next = state->allocated_objects;
        state->allocated_objects = this;
    }

    StringObject::StringObject(State* state, const StringInfo& string_info)
        : Object(state), state(state), text(string_info.text), length(string_info.length), hash(string_info.hash)
    {
    }

    StringObject::~StringObject()
    {
        deallocate(state, const_cast<char*>(text), length + 1);
    }

    StringObject* StringObject::from_text(State* state, std::string_view source_string)
    {
        StringInfo string_info;
        string_info.hash = std::hash<std::string_view>()(source_string);
        string_info.text = source_string.data();
        string_info.length = source_string.length();

        auto it = state->interned_strings.find(string_info);
        if(it != state->interned_strings.end()) return *it;

        char* text = allocate_array<char>(state, source_string.length() + 1);
        std::memcpy(text, source_string.data(), source_string.length());
        text[source_string.length()] = '\0';
        string_info.text = text;

        StringObject* str = construct_ptr<StringObject>(state, state, string_info);
        state->interned_strings.insert(str);

        return str;
    }
}