#include "VmStack.hpp"

#include <cassert>

namespace wf
{
    void VmStack::push_frame(BytecodeObject* function, std::uint64_t saved_ip,std::size_t return_idx)
    {
        std::size_t offset = 0;

        if(m_active_frame_count > 0)
        {
            offset = get_top_frame().frame_offset + get_top_frame().reserved_register_count;
        }

        m_active_frame_count++;
        get_top_frame().function = function;
        get_top_frame().saved_ip = saved_ip;
        get_top_frame().return_idx = return_idx;

        get_top_frame().reserved_register_count = 0;
        get_top_frame().frame_offset = offset;
    }

    void VmStack::pop_frame()
    {
        m_active_frame_count--;
    }

    BytecodeObject* VmStack::get_frame_function() const
    {
        return get_top_frame().function;
    }

    std::size_t VmStack::get_saved_ip() const
    {
        return get_top_frame().saved_ip;
    }

    std::size_t VmStack::get_return_idx() const
    {
        return get_top_frame().return_idx;
    }

    void VmStack::reserve(std::size_t count)
    {
        get_top_frame().reserved_register_count += count;
    }

    void VmStack::release(std::size_t count)
    {
        assert(count <= get_top_frame().reserved_register_count);
        get_top_frame().reserved_register_count -= count;
    }

    std::size_t VmStack::get_reserved_register_count() const
    {
        return get_top_frame().reserved_register_count;
    }

    Value& VmStack::index(std::size_t position)
    {
        return m_registers[get_top_frame().frame_offset + position];
    }

    const Value& VmStack::index(std::size_t position) const
    {
        return m_registers[get_top_frame().frame_offset + position];
    }

    StackFrame& VmStack::get_top_frame()
    {
        return m_frames[m_active_frame_count - 1];
    }

    const StackFrame& VmStack::get_top_frame() const
    {
        return m_frames[m_active_frame_count - 1];
    }

}