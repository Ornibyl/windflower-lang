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

    struct CompileInfo
    {
        std::string_view name;
        std::string_view source;
    };

    class Environment
    {
    public:
        Environment(const EnvironmentCreateInfo& create_info);
        Environment(Environment&& other);
        Environment& operator=(Environment&& other);
        ~Environment();

        void reserve(std::size_t count);
        void release(std::size_t count);
        std::size_t get_reserved_register_count() const;

        bool compile(std::size_t idx, const CompileInfo& compile_info);

        void call(std::size_t idx, std::size_t return_idx);
        void call(std::size_t idx);

        void store_int(std::size_t idx, Int value);
        void store_uint(std::size_t idx, UInt value);
        void store_float(std::size_t idx, Float value);
        void store_bool(std::size_t idx, bool value);
        void store_string(std::size_t idx, std::string_view value);

        Int get_int(std::size_t idx) const;
        UInt get_uint(std::size_t idx) const;
        Float get_float(std::size_t idx) const;
        bool get_bool(std::size_t idx) const;
        std::string_view get_string(std::size_t idx) const;
    private:
        State* m_state;
    };
}

#endif