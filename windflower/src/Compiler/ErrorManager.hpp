#ifndef WF_ERROR_MANAGER_HPP
#define WF_ERROR_MANAGER_HPP

#include "Token.hpp"
#include "Utils/Format.hpp"

template<>
struct fmt::formatter<wf::SourcePosition>
{
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const wf::SourcePosition& position, FormatContext& ctx)
    {
        return fmt::format_to(ctx.out(), "(ln {}, col {})", position.line, position.column);
    }
};

namespace wf
{
    class ErrorManager
    {
    public:
        ErrorManager(State* state)
            : m_message(state)
        {
        }

        template<typename... Args>
        void push_error(const SourcePosition& position, fmt::format_string<Args...> format_str, Args&&... args)
        {
            m_has_errors = true;
            fmt::format_to(std::back_inserter(m_message), "\n{}{} Error: ", position.source_name, position);
            fmt::format_to(std::back_inserter(m_message), format_str, std::forward<Args>(args)...);
        }

        bool has_errors() const { return m_has_errors; }
        const String& get_message() const { return m_message; }
    private:
        bool m_has_errors = false;
        String m_message;
    };
}

#endif