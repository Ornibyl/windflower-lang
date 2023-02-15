#include "Allocate.hpp"

#include "State.hpp"

namespace wf
{
    void* allocate(State* state, std::size_t size)
    {
        void* ptr = state->allocator(nullptr, 0, size);

        if(ptr == nullptr)
        {
            throw std::bad_alloc();
        }

        return ptr;
    }

    void deallocate(State* state, void* ptr, std::size_t size)
    {
        state->allocator(ptr, size, 0);
    }

    void* reallocate(State* state, void* ptr, std::size_t old_size, std::size_t new_size)
    {
        return state->allocator(ptr, old_size, new_size);
    }
}