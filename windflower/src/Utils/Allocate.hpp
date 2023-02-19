#ifndef WF_ALLOCATE_HPP
#define WF_ALLOCATE_HPP

#include <new>

#include "Windflower/Windflower.hpp"

namespace wf
{
    template<typename T>
    concept has_polymorphic_size_enable = requires(T* t)
    {
        { T::polymorphic_size_enable } -> std::same_as<std::true_type>;
        { t->get_class_size() } -> std::same_as<std::size_t>;
    };

    #define WF_POLYMORPHIC_SIZING                                                       \
        virtual std::size_t get_class_size() const noexcept { return sizeof(*this); }

    void* allocate(State* state, std::size_t size);
    void deallocate(State* state, void* ptr, std::size_t size);
    void* reallocate(State* state, void* ptr, std::size_t old_size, std::size_t new_size);

    template<typename T, typename... Args>
    T* construct_ptr(State* state, Args&&... args)
    {
        T* ptr = static_cast<T*>(allocate(state, sizeof(T)) );
        try
        {
            new(ptr) T(std::forward<Args>(args)...);
        }
        catch(...)
        {
            deallocate(state, ptr, sizeof(T));
            throw;
        }
        return ptr;
    }

    template<typename T, typename... Args>
    void destruct_ptr(State* state, T* ptr)
    {
        if constexpr(std::is_polymorphic_v<T>)
        {
            static_assert(has_polymorphic_size_enable<T>, "A polymorphic class must enable polymorphic sizing.");

            std::size_t size = ptr->get_class_size();

            ptr->~T();
            deallocate(state, state, size);
        }
        else
        {
            ptr->~T();
            deallocate(state, state, sizeof(T));
        }
    }

    template<typename T>
    T* allocate_array(State* state, std::size_t count)
    {
        return static_cast<T*>(allocate(state, sizeof(T) * count));
    }

    template<typename T>
    void deallocate_array(State* state, T* ptr, std::size_t count)
    {
        deallocate(state, ptr, sizeof(T) * count);
    }

    template<typename T>
    T* resize_array(State* state, T* ptr, std::size_t old_count, std::size_t new_count)
    {
        return static_cast<T*>(reallocate(state, ptr, sizeof(T) * old_count, sizeof(T) * new_count));
    }

    template<typename T>
    class StdAllocator
    {
    public:
        using value_type = T;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;

        StdAllocator(State* state) noexcept
            : m_state(state)
        {
        }

        bool operator==(const StdAllocator<T>&) const noexcept = default;
        auto operator<=>(const StdAllocator<T>&) const noexcept = default;

        T* allocate(size_type size)
        {
            return allocate_array<T>(m_state, size);
        }

        void deallocate(T* ptr, size_type size)
        {
            deallocate_array(m_state, ptr, size);
        }

    private:
        State* m_state;
    };
}

#endif