#ifndef WF_TOKEN_HPP
#define WF_TOKEN_HPP

#include <string_view>

namespace wf
{
    struct SourcePosition
    {
        SourcePosition() = default;
        SourcePosition(std::string_view source_name, std::uint32_t line, std::uint32_t column)
            : source_name(source_name), line(line), column(column)
        {
        }

        std::string_view source_name;
        std::uint32_t line = 0;
        std::uint32_t column = 0;
    };

    class Token
    {
    public:
        enum class Type
        {
            TT_EOF, ERROR,

            INT_CONSTANT, FLOAT_CONSTANT,
            PLUS, MINUS, STAR, SLASH, PERCENT,
            LEFT_PAREN, RIGHT_PAREN,

            TT_COUNT,
        };

        Token() = default;
        Token(Type type, const SourcePosition& position, std::string_view text)
            : m_type(type), m_position(position), m_text(text)
        {
        }

        Type get_type() const { return m_type; }
        SourcePosition get_position() const { return m_position; }
        std::string_view get_text() const { return m_text; }
    private:
        Type m_type;
        SourcePosition m_position;
        std::string_view m_text;
    };
}

#endif