#ifndef WF_SCOPED_PTR_HPP
#define WF_SCOPED_PTR_HPP

#include "Allocate.hpp"

#include <memory>

namespace wf
{
    template<typename T>
    class ScopedPtrDeleter
    {
    public:
        ScopedPtrDeleter(State* state)
            : m_state(state)
        {
        }

        ScopedPtrDeleter(const ScopedPtrDeleter&) = default;

        template<typename U>
        ScopedPtrDeleter(const ScopedPtrDeleter<U>& other)
            : m_state(other.get_state())
        {
        }

        void operator()(T* ptr) const
        {
            destruct_ptr(m_state, ptr);
        }

        template<typename U>
        void operator()(U* ptr) const
        {
            destruct_ptr(m_state, ptr);
        }

        State* get_state() const { return m_state; }

    private:
        State* m_state;
    };

    template<typename T>
    using ScopedPtr = std::unique_ptr<T, ScopedPtrDeleter<T>>;

    template<typename T, typename... Args>
    ScopedPtr<T> construct_scoped(State* state, Args&&... args)
    {
        return ScopedPtr<T>(new T(std::forward<Args>(args)...), state);
    }
}

#endif