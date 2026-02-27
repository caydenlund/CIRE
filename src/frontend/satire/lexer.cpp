#include "lexer.hpp"

#include <cctype>
#include <stdexcept>
#include <unordered_map>

namespace frontend::satire {

    static const std::unordered_map<std::string, TokenType> keywords = {
        {"INPUTS", TokenType::INPUTS},
        {"OUTPUTS", TokenType::OUTPUTS},
        {"CONSTRAINTS", TokenType::CONSTRAINTS},
        {"EXPRS", TokenType::EXPRS},
        {"if", TokenType::IF},
        {"then", TokenType::THEN},
        {"else", TokenType::ELSE},
        {"endif", TokenType::ENDIF},
        {"fl16", TokenType::FL16},
        {"fl32", TokenType::FL32},
        {"fl64", TokenType::FL64},
        {"int", TokenType::INT},
        {"sin", TokenType::SIN},
        {"cos", TokenType::COS},
        {"tan", TokenType::TAN},
        {"sinh", TokenType::SINH},
        {"cosh", TokenType::COSH},
        {"tanh", TokenType::TANH},
        {"asin", TokenType::ASIN},
        {"acos", TokenType::ACOS},
        {"atan", TokenType::ATAN},
        {"sqrt", TokenType::SQRT},
        {"exp", TokenType::EXP},
        {"log", TokenType::LOG},
        {"fma", TokenType::FMA},
    };

    Lexer::Lexer(std::string source) : _source(std::move(source)) {}

    char Lexer::advance() {
        if (isAtEnd()) return '\0';
        return _source[_current++];
    }

    char Lexer::peek_char() {
        if (isAtEnd()) return '\0';
        return _source[_current];
    }

    bool Lexer::match(char expected) {
        if (isAtEnd()) return false;
        if (_source[_current] != expected) return false;
        _current++;
        return true;
    }

    void Lexer::skipWhitespace() {
        while (!isAtEnd()) {
            char c = peek_char();
            if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
                if (c == '\n') {
                    _line++;
                    _lineStart = _current + 1;
                }
                advance();
            } else if (c == '#') {
                skipComment();
            } else {
                break;
            }
        }
    }

    void Lexer::skipComment() {
        // Skip until end of line
        while (!isAtEnd() && peek_char() != '\n') {
            advance();
        }
    }

    Token Lexer::makeToken(TokenType type) {
        return Token{type, "", 0.0, static_cast<int>(_line), static_cast<int>(_current - _lineStart)};
    }

    Token Lexer::makeToken(TokenType type, double value) {
        return Token{type, "", value, static_cast<int>(_line), static_cast<int>(_current - _lineStart)};
    }

    Token Lexer::number() {
        size_t start = _current - 1;

        // Integer part or first digit
        while (!isAtEnd() && std::isdigit(peek_char())) {
            advance();
        }

        // Fractional part
        if (!isAtEnd() && peek_char() == '.' && _current + 1 < _source.size() &&
            std::isdigit(_source[_current + 1])) {
            advance();  // consume '.'
            while (!isAtEnd() && std::isdigit(peek_char())) {
                advance();
            }
        }

        // Exponent part
        if (!isAtEnd() && (peek_char() == 'e' || peek_char() == 'E')) {
            advance();  // consume 'e' or 'E'
            if (!isAtEnd() && (peek_char() == '+' || peek_char() == '-')) {
                advance();  // consume sign
            }
            while (!isAtEnd() && std::isdigit(peek_char())) {
                advance();
            }
        }

        std::string numStr = _source.substr(start, _current - start);
        double value = std::stod(numStr);

        Token token = makeToken(TokenType::NUMBER, value);
        token.text = numStr;
        return token;
    }

    Token Lexer::identifier() {
        size_t start = _current - 1;

        while (!isAtEnd()) {
            char c = peek_char();
            if (std::isalnum(c) || c == '_') {
                advance();
            } else {
                break;
            }
        }

        std::string text = _source.substr(start, _current - start);

        // Check if it's a keyword
        auto it = keywords.find(text);
        if (it != keywords.end()) {
            return makeToken(it->second);
        }

        Token token = makeToken(TokenType::IDENTIFIER);
        token.text = text;
        return token;
    }

    Token Lexer::nextToken() {
        if (_peeked) {
            Token token = *_peeked;
            _peeked = std::nullopt;
            return token;
        }

        skipWhitespace();

        if (isAtEnd()) {
            return makeToken(TokenType::END_OF_FILE);
        }

        char c = advance();

        // Numbers
        if (std::isdigit(c)) {
            return number();
        }

        // Identifiers and keywords
        if (std::isalpha(c) || c == '_') {
            return identifier();
        }

        // Single-character tokens
        switch (c) {
            case '(': return makeToken(TokenType::LPAREN);
            case ')': return makeToken(TokenType::RPAREN);
            case '{': return makeToken(TokenType::LBRACE);
            case '}': return makeToken(TokenType::RBRACE);
            case ',': return makeToken(TokenType::COMMA);
            case ';': return makeToken(TokenType::SEMICOLON);
            case ':': return makeToken(TokenType::COLON);
            case '+': return makeToken(TokenType::PLUS);
            case '-': return makeToken(TokenType::MINUS);
            case '*': return makeToken(TokenType::STAR);
            case '/': return makeToken(TokenType::SLASH);

            case '=':
                if (match('=')) {
                    return makeToken(TokenType::EQ);
                }
                return makeToken(TokenType::ASSIGN);

            case '!':
                if (match('=')) {
                    return makeToken(TokenType::NEQ);
                }
                return makeToken(TokenType::NOT);

            case '<':
                if (match('=')) {
                    return makeToken(TokenType::LEQ);
                }
                return makeToken(TokenType::LT);

            case '>':
                if (match('=')) {
                    return makeToken(TokenType::GEQ);
                }
                return makeToken(TokenType::GT);

            case '&':
                if (match('&')) {
                    return makeToken(TokenType::AND);
                }
                break;

            case '|':
                if (match('|')) {
                    return makeToken(TokenType::OR);
                }
                break;
        }

        throw std::runtime_error("Unexpected character '" + std::string(1, c) + "' at line " +
                                std::to_string(_line));
    }

    Token Lexer::peek() {
        if (!_peeked) {
            _peeked = nextToken();
        }
        return *_peeked;
    }

}  // namespace frontend::satire
