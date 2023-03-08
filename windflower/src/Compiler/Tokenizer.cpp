#include "Tokenizer.hpp"
#include "Utils/Array.hpp"

#include <cctype>

namespace wf
{
    static auto keyword_to_string = arr_from_designators<std::string_view, static_cast<std::size_t>(Token::Type::TT_COUNT), Token::Type>({
        // While not technically a keyword, its easier to tokenize if we assume its one.
        { Token::Type::UNDERSCORE,  "_"      },

        { Token::Type::KW_VOID,     "Void"      },
        { Token::Type::KW_INT,      "Int"       },
        { Token::Type::KW_FLOAT,    "Float"     },

        { Token::Type::KW_VAR,      "var"       },
        { Token::Type::KW_EXTERN,   "extern"    },

        { Token::Type::KW_RETURN,   "return"    },
    });

    Tokenizer::Tokenizer(std::string_view name, std::string_view source)
        : m_begin(source.data()), m_end(source.data() + source.length()), m_current(m_begin),
            m_position(name, 1, 1)
    {
    }

    Token Tokenizer::next()
    {
        skip_whitespace();
        m_begin = m_current;
        if(is_finished()) return make_token(m_position, Token::Type::TT_EOF);

        SourcePosition start_position = m_position;

        if(std::isdigit(peek()))
        {
            while(std::isdigit(peek())) advance();
            if(peek() != '.') return make_token(start_position, Token::Type::INT_CONSTANT);
            advance();

            while(std::isdigit(peek())) advance();
            return make_token(start_position, Token::Type::FLOAT_CONSTANT);
        }

        if(std::isalpha(peek()) || peek() == '_')
        {
            switch(peek())
            {
                case '_': return finish_keyword(start_position, Token::Type::UNDERSCORE);
                case 'e': return finish_keyword(start_position, Token::Type::KW_EXTERN);
                case 'F': return finish_keyword(start_position, Token::Type::KW_FLOAT);
                case 'I': return finish_keyword(start_position, Token::Type::KW_INT);
                case 'r': return finish_keyword(start_position, Token::Type::KW_RETURN);
                case 'v':
                    advance();
                    switch(peek())
                    {
                        case 'a': return finish_keyword(start_position, Token::Type::KW_VAR);
                        case 'o': return finish_keyword(start_position, Token::Type::KW_VOID);
                        default:
                            break;
                    }
                default:
                    break;
            }

            while(std::isalpha(peek()) || std::isdigit(peek()) || peek() == '_') advance();
            return make_token(start_position, Token::Type::IDENTIFIER);
        }

        switch(peek())
        {
            case '\n':
                advance();
                return make_token(start_position, Token::Type::NEWLINE);
            case '+':
                advance();
                return make_token(start_position, Token::Type::PLUS);
            case '-':
                advance();
                if(peek() == '>')
                {
                    advance();
                    return make_token(start_position, Token::Type::ARROW);
                }
                return make_token(start_position, Token::Type::MINUS);
            case '*':
                advance();
                return make_token(start_position, Token::Type::STAR);
            case '/':
                advance();
                return make_token(start_position, Token::Type::SLASH);
            case '%':
                advance();
                return make_token(start_position, Token::Type::PERCENT);
            case '(':
                advance();
                return make_token(start_position, Token::Type::LEFT_PAREN);
            case ')':
                advance();
                return make_token(start_position, Token::Type::RIGHT_PAREN);
            case ',':
                advance();
                return make_token(start_position, Token::Type::COMMA);
            case ':':
                advance();
                if(peek() == '=')
                {
                    advance();
                    return make_token(start_position, Token::Type::COLON_EQUALS);
                }

                return make_token(start_position, Token::Type::COLON);
            default:
                break;
        }

        advance();
        return Token(Token::Type::ERROR, start_position, "Unknown character.");
    }

    void Tokenizer::advance()
    {
        if(is_finished()) return;

        m_position.column++;
        if(peek() == '\n')
        {
            m_position.line++;
            m_position.column = 1;
        }
        m_current++;
    }

    char Tokenizer::peek()
    {
        if(is_finished()) return '\0';
        return *m_current;
    }

    char Tokenizer::peek_next()
    {
        if(is_finished()) return '\0';
        if(m_current + 1 == m_end) return '\0';
        return m_current[1];
    }

    Token Tokenizer::make_token(SourcePosition position, Token::Type type)
    {
        return Token(type, position, { m_begin, static_cast<std::size_t>(m_current - m_begin) });
    }

    Token Tokenizer::finish_keyword(SourcePosition position, Token::Type type)
    {
        const std::string_view keyword_text = keyword_to_string[static_cast<std::size_t>(type)];

        bool is_keyword = true;
        std::size_t i = 0;

        while(std::isalpha(peek()) || std::isdigit(peek()) || peek() == '_')
        {
            if(i < keyword_text.length() && is_keyword && peek() != keyword_text[i])
            {
                is_keyword = false;
            }

            advance();
            i++;
        }

        if(i != keyword_text.length())
        {
            is_keyword = false;
        }

        return make_token(position, is_keyword? type : Token::Type::IDENTIFIER);
    }

    bool Tokenizer::is_finished()
    {
        return m_current == m_end;
    }

    void Tokenizer::skip_whitespace()
    {
        while(true)
        {
            switch(peek())
            {
                case '-':
                    if(peek_next() == '-')
                    {
                        while(!is_finished() && peek() != '\n')
                        {
                            advance();
                        }
                        break;
                    }
                    return;
                case '\n':
                    if(!m_newline_ignore) return;
                case ' ':
                case '\t':
                case '\v':
                case '\f':
                case '\r':
                    advance();
                    break;
                default:
                    return;
            }
        }
    }
}