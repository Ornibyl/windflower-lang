#include <Windflower/Windflower.hpp>

#include <cstdlib>

namespace wftool
{
    class MallocAllocator : public wf::Allocator
    {
    public:
        void* operator()(void* buffer, std::size_t old_size, std::size_t new_size) noexcept final
        {
            (void)old_size;
            if(new_size == 0)
            {
                std::free(buffer);
                return nullptr;
            }
            return std::realloc(buffer, new_size);
        }
    };
}

int main(int argc, const char* argv[])
{
    (void)argc;
    (void)argv;

    wftool::MallocAllocator allocator;

    wf::EnvironmentCreateInfo create_info = {
        .allocator = &allocator
    };

    wf::Environment env(create_info);
    env.push(10);
    
}