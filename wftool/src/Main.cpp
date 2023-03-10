#include <Windflower/Windflower.hpp>

#include <cstdlib>
#include <filesystem>

#include <iostream>
#include <fstream>
#include <sstream>

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

    void compile_from_file(wf::Environment& env, std::size_t idx, std::string_view path)
    {
        std::string file_text;
        {
            std::ifstream file(path);
            if(!file.is_open())
            {
                std::cerr << "Could not open file '" << path << "'.\n";
                std::exit(EXIT_FAILURE);
            }

            std::ostringstream file_text_stream;
            file_text_stream << file.rdbuf();
            file.close();
            file_text = file_text_stream.str();
        }

        wf::CompileInfo compile_info = {
            .name = path,
            .source = file_text
        };

        if(!env.compile(idx, compile_info))
        {
            std::cerr << "Could not compile file '" << path << "'.\n";
            std::cerr << env.get_string(idx) << "\n";
            std::exit(EXIT_FAILURE);
        }
    }

    wf::ReturnState print_int(wf::Environment& env)
    {
        wf::Int value = env.get_int(0);
        std::cout << value << "\n";
        return wf::ReturnState::NO_VALUE;
    }

    wf::ReturnState print_float(wf::Environment& env)
    {
        wf::Float value = env.get_float(0);
        std::cout << value << "\n";
        return wf::ReturnState::NO_VALUE;
    }

    void register_io_funcs(wf::Environment& env)
    {
        env.register_native_func("print_int(_:", &print_int);
        env.register_native_func("print_float(_:", &print_float);
    }
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
    wftool::register_io_funcs(env);
    env.reserve(2);

    wftool::compile_from_file(env, 0, "TestScripts/Main.wf");

    env.disassemble_bytecode(1, 0);
    std::cout << env.get_string(1) << "\n";

    env.call(0);
}