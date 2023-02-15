#include "State.hpp"

namespace wf
{
    State::State(const EnvironmentCreateInfo& create_info)
        : allocator(*create_info.allocator)
    {
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

    void Environment::push(std::size_t count)
    {
        m_state->stack.push(count);
    }

    void Environment::pop(std::size_t count)
    {
        m_state->stack.pop(count);
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

    Int Environment::get_int(std::size_t idx)
    {
        return static_cast<Int>(m_state->stack.index(idx).as_int);
    }

    UInt Environment::get_uint(std::size_t idx)
    {
        return m_state->stack.index(idx).as_int;
    }

    Float Environment::get_float(std::size_t idx)
    {
        return m_state->stack.index(idx).as_float;
    }

    bool Environment::get_bool(std::size_t idx)
    {
        return m_state->stack.index(idx).as_bool;
    }

}