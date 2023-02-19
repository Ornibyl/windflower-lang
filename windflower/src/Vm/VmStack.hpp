#ifndef WF_VM_STACK_HPP
#define WF_VM_STACK_HPP

#include <array>
#include <cstddef>

#include "Value.hpp"
#include "Vm/Object.hpp"

namespace wf
{
    struct StackFrame
    {
        BytecodeObject* function = nullptr;
        std::uint64_t saved_ip;
        std::size_t return_idx;

        std::size_t reserved_register_count = 0;
        std::size_t frame_offset = 0;
    };

    class VmStack
    {
    public:
        static constexpr std::size_t REGISTER_COUNT = 1024 * 128;
        static constexpr std::size_t MAX_FRAME_COUNT = 256;

        void push_frame(BytecodeObject* function, std::uint64_t saved_ip, std::size_t return_idx);
        void pop_frame();

        BytecodeObject* get_frame_function() const;
        std::size_t get_saved_ip() const;
        std::size_t get_return_idx() const;

        void reserve(std::size_t count);
        void release(std::size_t count);
        std::size_t get_reserved_register_count() const;

        Value& index(std::size_t position);
        const Value& index(std::size_t position) const;

        decltype(auto) begin() { return m_registers.begin(); }
        decltype(auto) begin() const { return m_registers.begin(); }
        decltype(auto) cbegin() const { return m_registers.cbegin(); }

        decltype(auto) rbegin() { return m_registers.rbegin(); }
        decltype(auto) rbegin() const { return m_registers.rbegin(); }
        decltype(auto) crbegin() const { return m_registers.crbegin(); }

        decltype(auto) end() { return m_registers.cend(); }
        decltype(auto) end() const { return m_registers.cend(); }
        decltype(auto) cend() const { return m_registers.cend(); }

        decltype(auto) rend() { return m_registers.rend(); }
        decltype(auto) rend() const { return m_registers.rend(); }
        decltype(auto) crend() const { return m_registers.crend(); }
    private:
        std::array<Value, REGISTER_COUNT> m_registers;
        std::array<StackFrame, MAX_FRAME_COUNT> m_frames;
        std::size_t m_active_frame_count = 0;

        StackFrame& get_top_frame();
        const StackFrame& get_top_frame() const;
    };
}

#endif