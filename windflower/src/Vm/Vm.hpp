#ifndef WF_VM_HPP
#define WF_VM_HPP

#include "VmStack.hpp"
#include "Utils/String.hpp"

namespace wf
{
    class VmError
    {
    public:
        VmError(const String& message)
            : m_message(message)
        {
        }

        const String& get_message() const
        {
            return m_message;
        }
    private:
        String m_message;
    };

    class Vm
    {
    public:
        Vm(State* state) : m_state(state) {};

        void call(std::size_t idx, std::size_t return_idx);
    private:
        State* m_state;

        std::uint64_t m_ip;

        void run();

        void error(const String& message);
    };
}

#endif