#ifndef WF_TOKENIZER_HPP
#define WF_TOKENIZER_HPP

#include "Compiler/Token.hpp"

namespace wf
{
    class Tokenizer
    {
    public:
        Tokenizer(std::string_view name, std::string_view source);

        Token next();
        void set_newline_ignore(bool value) { m_newline_ignore = value; }
    private:
        bool m_newline_ignore;

        const char* m_begin;
        const char* m_end;
        const char* m_current;

        SourcePosition m_position;

        void advance();
        char peek();
        char peek_next();

        Token make_token(SourcePosition position, Token::Type type);
        Token finish_keyword(SourcePosition position, Token::Type type);

        bool is_finished();
        void skip_whitespace();
    };
}

#endif