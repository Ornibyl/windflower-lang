#ifndef WF_TOKEN_HPP
#define WF_TOKEN_HPP

#include <unordered_map>
#include <string_view>
#include <limits>

namespace wf
{
    struct SourcePosition
    {
        constexpr SourcePosition() = default;
        constexpr SourcePosition(std::string_view source_name, std::uint32_t line, std::uint32_t column)
            : source_name(source_name), line(line), column(column)
        {
        }

        std::string_view source_name;
        std::uint32_t line = 0;
        std::uint32_t column = 0;

        static consteval SourcePosition no_pos()
        {
            return { "", std::numeric_limits<std::uint32_t>::max(), std::numeric_limits<std::uint32_t>::max() };
        }

        constexpr bool operator==(const SourcePosition& other) const = default;
    };

    class Token
    {
    public:
        enum class Type
        {
            TT_EOF, ERROR, NEWLINE,

            IDENTIFIER, UNDERSCORE,
            INT_CONSTANT, FLOAT_CONSTANT,
            PLUS, MINUS, STAR, SLASH, PERCENT,
            COMMA, COLON, COLON_EQUALS, ARROW,
            LEFT_PAREN, RIGHT_PAREN,

            KW_VOID, KW_INT, KW_FLOAT,
            KW_VAR, KW_EXTERN,
            KW_RETURN,

            TT_COUNT,
        };

        constexpr Token() noexcept
            : m_type(Token::Type::TT_EOF)
        {
        }

        constexpr Token(Type type, const SourcePosition& position, std::string_view text) noexcept
            : m_type(type), m_position(position), m_text(text)
        {
        }

        constexpr Type get_type() const noexcept { return m_type; }
        constexpr SourcePosition get_position() const noexcept { return m_position; }
        constexpr std::string_view get_text() const noexcept { return m_text; }

        constexpr bool is_keyword() const noexcept
        {
            return get_type() == Type::KW_VOID || get_type() == Type::KW_INT || get_type() == Type::KW_FLOAT
                || get_type() == Type::KW_VAR || get_type() == Type::KW_EXTERN
                || get_type() == Type::KW_RETURN;
        }
    private:
        Type m_type;
        SourcePosition m_position;
        std::string_view m_text;
    };
}

#endif