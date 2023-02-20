#ifndef WF_OBJECT_HPP
#define WF_OBJECT_HPP

#include "Utils/Array.hpp"
#include "Instructions.hpp"
#include "Value.hpp"

namespace wf
{
    struct BytecodeLineInfo
    {
        std::size_t offset;
        std::uint16_t line;
    };

    struct Object
    {
        using polymorphic_size_enable = std::true_type;
        WF_POLYMORPHIC_SIZING

        enum class Type
        {
            BYTECODE,
        };

        Object(State* state);

        Object* next = nullptr;

        virtual ~Object() = default;
    };

    struct StringInfo
    {
        const char* text;
        std::size_t length;
        std::size_t hash;
    };

    struct StringObject : public Object
    {
        WF_POLYMORPHIC_SIZING

        StringObject(State* state, const StringInfo& string_info);

        ~StringObject();

        State* const state;
        const char* const text;
        const std::size_t length;
        const std::size_t hash;

        static StringObject* from_text(State* state, std::string_view source_string);
    };

    struct BytecodeObject : public Object
    {
        WF_POLYMORPHIC_SIZING

        BytecodeObject(State* state)
            : Object(state), line_info(state), code(state), constants(state)
        {
        }

        DynamicArray<BytecodeLineInfo> line_info;
        DynamicArray<Instruction> code;
        DynamicArray<Value> constants;
    };
}

namespace std
{
    template<>
    struct hash<wf::StringObject*>
    {
        using is_transparent = void;

        size_t operator()(wf::StringObject* value) const
        {
            return value->hash;
        }

        size_t operator()(const wf::StringInfo& value) const
        {
            return value.hash;
        }
    };
}

#endif