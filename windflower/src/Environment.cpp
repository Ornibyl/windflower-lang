#include "State.hpp"

#include "Utils/Format.hpp"
#include "Vm/Object.hpp"
#include "Utils/Allocate.hpp"
#include "Compiler/Tokenizer.hpp"

namespace wf
{
    State::State(const EnvironmentCreateInfo& create_info)
        : allocator(*create_info.allocator), interned_strings(this), vm(this)
    {
        // Global frame
        stack.push_frame(nullptr, 0, 0);
    }

    State::~State()
    {
        stack.pop_frame();
        Object* obj = allocated_objects;
        while(obj != nullptr)
        {
            Object* deleted_object = obj;
            obj = obj->next;
            destruct_ptr(this, deleted_object);
        }
    }

    Environment::Environment(const EnvironmentCreateInfo& create_info)
    {
        m_state = static_cast<State*>( (*create_info.allocator)(nullptr, 0, sizeof(State)) );
        new(m_state) State(create_info);
    }

    Environment::Environment(Environment&& other)
        : m_state(other.m_state)
    {
        other.m_state = nullptr;
    }

    Environment& Environment::operator=(Environment&& other)
    {
        Allocator& allocator = m_state->allocator;
        m_state->~State();
        allocator(m_state, sizeof(State), 0);

        m_state = other.m_state;
        return *this;
    }

    Environment::~Environment()
    {
        Allocator& allocator = m_state->allocator;
        m_state->~State();
        allocator(m_state, sizeof(State), 0);
    }

    void Environment::reserve(std::size_t count)
    {
        m_state->stack.reserve(count);
    }

    void Environment::release(std::size_t count)
    {
        m_state->stack.release(count);
    }

    std::size_t Environment::get_reserved_register_count() const
    {
        return m_state->stack.get_reserved_register_count();
    }

    bool Environment::compile(std::size_t idx, const CompileInfo& compile_info)
    {
        DynamicArray<Token> tokens(m_state);

        Tokenizer tokenizer(compile_info.name, compile_info.source);

        while(true)
        {
            const Token& token = tokens.emplace_back(tokenizer.next());
            if(token.get_type() == Token::Type::TT_EOF) break;
        }

        m_state->stack.index(idx) = StringObject::from_text(
            m_state,
            format(m_state, "{}(\?\?\?) Error: Compilation pipeline is incomplete.", compile_info.name)
        );
        return false;
    }

    void Environment::call(std::size_t idx, std::size_t return_idx)
    {
        m_state->vm.call(idx, return_idx);
    }

    void Environment::call(std::size_t idx)
    {
        m_state->vm.call(idx, 0);
    }

    void Environment::store_int(std::size_t idx, Int value)
    {
        m_state->stack.index(idx).as_int = static_cast<UInt>(value);
    }

    void Environment::store_uint(std::size_t idx, UInt value)
    {
        m_state->stack.index(idx).as_int = value;
    }

    void Environment::store_float(std::size_t idx, Float value)
    {
        m_state->stack.index(idx).as_float = value;
    }

    void Environment::store_bool(std::size_t idx, bool value)
    {
        m_state->stack.index(idx).as_bool = value;
    }

    void Environment::store_string(std::size_t idx, std::string_view value)
    {
        m_state->stack.index(idx).as_object = StringObject::from_text(m_state, value);
    }

    Int Environment::get_int(std::size_t idx) const
    {
        return static_cast<Int>(m_state->stack.index(idx).as_int);
    }

    UInt Environment::get_uint(std::size_t idx) const
    {
        return m_state->stack.index(idx).as_int;
    }

    Float Environment::get_float(std::size_t idx) const
    {
        return m_state->stack.index(idx).as_float;
    }

    bool Environment::get_bool(std::size_t idx) const
    {
        return m_state->stack.index(idx).as_bool;
    }

    std::string_view Environment::get_string(std::size_t idx) const
    {
        StringObject* str = m_state->stack.index(idx).as_string();
        return { str->text, str->length };
    }

}