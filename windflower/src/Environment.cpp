#include "State.hpp"

#include "Vm/Object.hpp"

namespace wf
{
    State::State(const EnvironmentCreateInfo& create_info)
        : allocator(*create_info.allocator), vm(this)
    {
    }

    Environment::Environment(const EnvironmentCreateInfo& create_info)
    {
        m_state = static_cast<State*>( (*create_info.allocator)(nullptr, 0, sizeof(State)) );
        new(m_state) State(create_info);

        // Global frame
        m_state->stack.push_frame(nullptr, 0, 0);
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
        m_state->stack.pop_frame();

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

    void Environment::compile(std::size_t idx, const CompileInfo& compile_info)
    {
        // Everything here is temporary
        (void)compile_info;

        BytecodeObject* bytecode = construct_ptr<BytecodeObject>(m_state, m_state);

        bytecode->constants.emplace_back(Value{ .as_int = 20 });
        bytecode->constants.emplace_back(Value{ .as_int = 5 });

        bytecode->code.emplace_back(Instruction(Opcode::RESERVE, 2));
        bytecode->code.emplace_back(Instruction(Opcode::LOAD_CONSTANT, 0, 0));
        bytecode->code.emplace_back(Instruction(Opcode::LOAD_CONSTANT, 1, 1));
        bytecode->code.emplace_back(Instruction(Opcode::ADD_INT, 0, 1));
        bytecode->code.emplace_back(Instruction(Opcode::MOVE, 1, 0));
        bytecode->code.emplace_back(Instruction(Opcode::RETURN, 2));

        m_state->stack.index(idx).as_object = bytecode;
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

}