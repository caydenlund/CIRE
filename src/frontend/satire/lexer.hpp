#pragma once

#include <string>
#include <vector>
#include <optional>

namespace frontend::satire {

    enum class TokenType {
        // Keywords
        INPUTS, OUTPUTS, CONSTRAINTS, EXPRS,
        IF, THEN, ELSE, ENDIF,

        // Types
        FL16, FL32, FL64, INT,

        // Operators
        PLUS, MINUS, STAR, SLASH,
        EQ, NEQ, LT, LEQ, GT, GEQ,
        AND, OR, NOT,
        ASSIGN,

        // Functions
        SIN, COS, TAN, SINH, COSH, TANH,
        ASIN, ACOS, ATAN,
        SQRT, EXP, LOG, FMA,

        // Delimiters
        LPAREN, RPAREN, LBRACE, RBRACE,
        COMMA, SEMICOLON, COLON,

        // Literals
        IDENTIFIER, NUMBER,

        // Special
        END_OF_FILE, NEWLINE
    };

    struct Token {
        TokenType type;
        std::string text;
        double numValue {0.0};
        int line {0};
        int col {0};
    };

    class Lexer {
    public:
        explicit Lexer(std::string source);

        Token nextToken();
        Token peek();
        bool isAtEnd() const { return _current >= _source.size(); }

    private:
        std::string _source;
        size_t _current {0};
        size_t _line {1};
        size_t _lineStart {0};
        std::optional<Token> _peeked;

        char advance();
        char peek_char();
        bool match(char expected);
        void skipWhitespace();
        void skipComment();

        Token makeToken(TokenType type);
        Token makeToken(TokenType type, double value);
        Token number();
        Token identifier();
    };

}  // namespace frontend::satire
