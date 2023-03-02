#include "Tokenizer.hpp"

#include <cctype>

namespace wf
{
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

        switch(peek())
        {
            case '+':
                advance();
                return make_token(start_position, Token::Type::PLUS);
            case '-':
                advance();
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