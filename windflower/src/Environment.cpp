#include "State.hpp"

#include "Utils/Format.hpp"
#include "Vm/Bytecode.hpp"
#include "Vm/Object.hpp"
#include "Utils/Allocate.hpp"

#include "Compiler/Parser.hpp"
#include "Compiler/Resolver.hpp"
#include "Compiler/CodeGen.hpp"

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
        BytecodeObject* result_code = construct_ptr<BytecodeObject>(m_state, m_state);
        m_state->stack.index(idx) = result_code;
        Parser parser(m_state, compile_info);
        Resolver resolver(m_state);
        CodeGen code_gen(m_state, result_code);

        Node* ast = parser.parse();
        if(ast == nullptr)
        {
            m_state->stack.index(idx) = StringObject::from_text(m_state, parser.get_error_message());
            return false;
        }

        Action* action_tree = resolver.resolve_ast(ast);

        if(action_tree == nullptr)
        {
            m_state->stack.index(idx) = StringObject::from_text(m_state, resolver.get_error_message());
            return false;
        }

        code_gen.generate(action_tree);

        return true;
    }

    void Environment::disassemble_bytecode(std::size_t return_idx, std::size_t idx)
    {
        m_state->stack.index(return_idx) = StringObject::from_text(m_state,
                disassemble_bytecode_object( m_state, m_state->stack.index(idx).as_bytecode() ));
    }

    TypeId Environment::get_bytecode_return_type(std::size_t idx)
    {
        return m_state->stack.index(idx).as_bytecode()->return_type;
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