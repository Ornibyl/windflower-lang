#ifndef WF_WINDFLOWER_HPP
#define WF_WINDFLOWER_HPP

#include <string_view>
#include <cstdint>

namespace wf
{
    struct State;

    using Int = int64_t;
    using UInt = std::uint64_t;
    using Float = double;

    class Allocator
    {
    public:
        virtual void* operator()(void* buffer, std::size_t old_size, std::size_t new_size) noexcept = 0;
    };

    struct EnvironmentCreateInfo
    {
        Allocator* allocator = nullptr;
    };

    class Environment
    {
    public:
        Environment(const EnvironmentCreateInfo& create_info);
        Environment(Environment&& other);
        Environment& operator=(Environment&& other);
        ~Environment();

        void push(std::size_t count);
        void pop(std::size_t count);

        void store_int(std::size_t idx, Int value);
        void store_uint(std::size_t idx, UInt value);
        void store_float(std::size_t idx, Float value);
        void store_bool(std::size_t idx, bool value);

        Int get_int(std::size_t idx);
        UInt get_uint(std::size_t idx);
        Float get_float(std::size_t idx);
        bool get_bool(std::size_t idx);
    private:
        State* m_state;
    };
}

#endif