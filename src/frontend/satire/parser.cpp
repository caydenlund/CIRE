#include "parser.hpp"

#include <stdexcept>
#include <iostream>

namespace frontend::satire {

    Parser::Parser(Lexer& lexer) : _lexer(lexer) {
        advance();
    }

    void Parser::advance() {
        _current = _lexer.nextToken();
    }

    bool Parser::check(TokenType type) {
        return _current.type == type;
    }

    bool Parser::match(TokenType type) {
        if (check(type)) {
            advance();
            return true;
        }
        return false;
    }

    void Parser::consume(TokenType type, const std::string& message) {
        if (!check(type)) {
            error(message + " (got " + _current.text + " at line " + std::to_string(_current.line) + ")");
        }
        advance();
    }

    void Parser::error(const std::string& message) {
        throw std::runtime_error("Parse error at line " + std::to_string(_current.line) + ": " + message);
    }

    graph::FloatPrec Parser::tokenToPrec(TokenType type) {
        switch (type) {
            case TokenType::FL16: return graph::FloatPrec::F16;
            case TokenType::FL32: return graph::FloatPrec::F32;
            case TokenType::FL64: return graph::FloatPrec::F64;
            default: error("Invalid precision type");
        }
    }

    graph::ComputationGraph Parser::parse() {
        // Parse INPUTS section
        consume(TokenType::INPUTS, "Expected 'INPUTS'");
        parseInputs();

        // Parse OUTPUTS section
        consume(TokenType::OUTPUTS, "Expected 'OUTPUTS'");
        parseOutputs();

        // Optional CONSTRAINTS section (skip for now)
        if (match(TokenType::CONSTRAINTS)) {
            consume(TokenType::LBRACE, "Expected '{'");
            // Skip constraints until we hit '}'
            int braceCount = 1;
            while (braceCount > 0 && !check(TokenType::END_OF_FILE)) {
                if (match(TokenType::LBRACE)) braceCount++;
                else if (match(TokenType::RBRACE)) braceCount--;
                else advance();
            }
        }

        // Parse EXPRS section
        consume(TokenType::EXPRS, "Expected 'EXPRS'");
        parseExprs();

        _graph.validate();
        return std::move(_graph);
    }

    void Parser::parseInputs() {
        consume(TokenType::LBRACE, "Expected '{' after INPUTS");

        while (!check(TokenType::RBRACE) && !check(TokenType::END_OF_FILE)) {
            // Parse: name type : (lower, upper);
            if (!check(TokenType::IDENTIFIER)) {
                error("Expected variable name in INPUTS");
            }

            std::string varName = _current.text;
            advance();

            // Parse type
            if (!check(TokenType::FL16) && !check(TokenType::FL32) && !check(TokenType::FL64)) {
                error("Expected type (fl16, fl32, fl64) for variable " + varName);
            }
            graph::FloatPrec prec = tokenToPrec(_current.type);
            advance();

            consume(TokenType::COLON, "Expected ':' after type");
            consume(TokenType::LPAREN, "Expected '(' for interval");

            // Parse lower bound
            if (!check(TokenType::NUMBER) && !check(TokenType::MINUS)) {
                error("Expected number for lower bound");
            }
            double lowerSign = 1.0;
            if (match(TokenType::MINUS)) {
                lowerSign = -1.0;
            }
            if (!check(TokenType::NUMBER)) {
                error("Expected number after minus");
            }
            double lower = lowerSign * _current.numValue;
            advance();

            consume(TokenType::COMMA, "Expected ',' in interval");

            // Parse upper bound
            if (!check(TokenType::NUMBER) && !check(TokenType::MINUS)) {
                error("Expected number for upper bound");
            }
            double upperSign = 1.0;
            if (match(TokenType::MINUS)) {
                upperSign = -1.0;
            }
            if (!check(TokenType::NUMBER)) {
                error("Expected number after minus");
            }
            double upper = upperSign * _current.numValue;
            advance();

            consume(TokenType::RPAREN, "Expected ')' after interval");
            consume(TokenType::SEMICOLON, "Expected ';' after input declaration");

            // Add input to graph
            graph::NodeId inputId = _graph.addInput(varName, prec);
            _symbolTable[varName] = inputId;
        }

        consume(TokenType::RBRACE, "Expected '}' after INPUTS");
    }

    void Parser::parseOutputs() {
        consume(TokenType::LBRACE, "Expected '{' after OUTPUTS");

        while (!check(TokenType::RBRACE) && !check(TokenType::END_OF_FILE)) {
            if (!check(TokenType::IDENTIFIER)) {
                error("Expected output variable name");
            }

            std::string outName = _current.text;
            advance();

            consume(TokenType::SEMICOLON, "Expected ';' after output name");

            // Remember output names to mark after parsing expressions
            _outputNames.push_back(outName);
        }

        consume(TokenType::RBRACE, "Expected '}' after OUTPUTS");
    }

    void Parser::parseExprs() {
        consume(TokenType::LBRACE, "Expected '{' after EXPRS");

        std::vector<std::string> outputNames;

        while (!check(TokenType::RBRACE) && !check(TokenType::END_OF_FILE)) {
            // Parse assignment: name [type] = expr;
            if (!check(TokenType::IDENTIFIER)) {
                error("Expected variable name in expression");
            }

            std::string varName = _current.text;
            advance();

            // Optional type annotation
            graph::FloatPrec prec = graph::FloatPrec::F64;  // default
            if (check(TokenType::FL16) || check(TokenType::FL32) || check(TokenType::FL64)) {
                prec = tokenToPrec(_current.type);
                advance();
            }

            // Optional rounding annotation (rnd16, rnd32, rnd64, etc.) - skip it
            if (check(TokenType::IDENTIFIER) && _current.text.substr(0, 3) == "rnd") {
                advance();
            }

            consume(TokenType::ASSIGN, "Expected '=' in assignment");

            // Parse expression
            graph::NodeId exprId = parseExpression();

            consume(TokenType::SEMICOLON, "Expected ';' after expression");

            // Store in symbol table
            _symbolTable[varName] = exprId;
        }

        consume(TokenType::RBRACE, "Expected '}' after EXPRS");

        // Mark outputs specified in OUTPUTS section
        for (const std::string& outName : _outputNames) {
            auto it = _symbolTable.find(outName);
            if (it == _symbolTable.end()) {
                error("Output variable '" + outName + "' not found in expressions");
            }
            _graph.markOutput(it->second);
        }

        // If no outputs were specified, mark all expressions as outputs (fallback)
        if (_outputNames.empty() && !_symbolTable.empty()) {
            for (const auto& [name, nodeId] : _symbolTable) {
                _graph.markOutput(nodeId);
            }
        }
    }

    graph::NodeId Parser::parseExpression() {
        return parseAdditive();
    }

    graph::NodeId Parser::parseAdditive() {
        graph::NodeId left = parseMultiplicative();

        while (check(TokenType::PLUS) || check(TokenType::MINUS)) {
            bool isPlus = check(TokenType::PLUS);
            advance();  // consume the operator
            graph::NodeId right = parseMultiplicative();

            if (isPlus) {
                left = _graph.addNode(graph::AddNode{left, right}, graph::FloatPrec::F64);
            } else {
                left = _graph.addNode(graph::SubNode{left, right}, graph::FloatPrec::F64);
            }
        }

        return left;
    }

    graph::NodeId Parser::parseMultiplicative() {
        graph::NodeId left = parseUnary();

        while (true) {
            if (match(TokenType::STAR)) {
                graph::NodeId right = parseUnary();
                left = _graph.addNode(graph::MulNode{left, right}, graph::FloatPrec::F64);
            } else if (match(TokenType::SLASH)) {
                graph::NodeId right = parseUnary();
                left = _graph.addNode(graph::DivNode{left, right}, graph::FloatPrec::F64);
            } else {
                break;
            }
        }

        return left;
    }

    graph::NodeId Parser::parseUnary() {
        if (match(TokenType::MINUS)) {
            graph::NodeId operand = parseUnary();
            return _graph.addNode(graph::NegNode{operand}, graph::FloatPrec::F64);
        }

        return parsePrimary();
    }

    graph::NodeId Parser::parsePrimary() {
        // Number
        if (check(TokenType::NUMBER)) {
            double value = _current.numValue;
            advance();
            return _graph.addNode(graph::ConstantNode{value, graph::FloatPrec::F64},
                                graph::FloatPrec::F64);
        }

        // Parenthesized expression
        if (match(TokenType::LPAREN)) {
            graph::NodeId expr = parseExpression();
            consume(TokenType::RPAREN, "Expected ')' after expression");
            return expr;
        }

        // Function calls
        if (check(TokenType::SIN) || check(TokenType::COS) || check(TokenType::TAN) ||
            check(TokenType::SQRT) || check(TokenType::EXP) || check(TokenType::LOG) ||
            check(TokenType::FMA) || check(TokenType::ASIN) || check(TokenType::ACOS) ||
            check(TokenType::ATAN) || check(TokenType::SINH) || check(TokenType::COSH) ||
            check(TokenType::TANH)) {

            TokenType funcType = _current.type;
            advance();
            return parseFunction(funcType);
        }

        // Variable reference
        if (check(TokenType::IDENTIFIER)) {
            std::string varName = _current.text;
            advance();

            auto it = _symbolTable.find(varName);
            if (it == _symbolTable.end()) {
                error("Undefined variable: " + varName);
            }

            return it->second;
        }

        error("Expected expression");
    }

    graph::NodeId Parser::parseFunction(TokenType funcType) {
        consume(TokenType::LPAREN, "Expected '(' after function name");

        if (funcType == TokenType::FMA) {
            // fma(a, b, c) = a * b + c
            graph::NodeId a = parseExpression();
            consume(TokenType::COMMA, "Expected ',' in fma");
            graph::NodeId b = parseExpression();
            consume(TokenType::COMMA, "Expected ',' in fma");
            graph::NodeId c = parseExpression();
            consume(TokenType::RPAREN, "Expected ')' after fma");

            return _graph.addNode(graph::FmaNode{a, b, c}, graph::FloatPrec::F64);
        }

        // Unary functions
        graph::NodeId arg = parseExpression();
        consume(TokenType::RPAREN, "Expected ')' after function argument");

        switch (funcType) {
            case TokenType::SIN:
                return _graph.addNode(graph::SinNode{arg}, graph::FloatPrec::F64);
            case TokenType::COS:
                return _graph.addNode(graph::CosNode{arg}, graph::FloatPrec::F64);
            case TokenType::SQRT:
                return _graph.addNode(graph::SqrtNode{arg}, graph::FloatPrec::F64);
            case TokenType::EXP:
                return _graph.addNode(graph::ExpNode{arg}, graph::FloatPrec::F64);
            case TokenType::LOG:
                return _graph.addNode(graph::LogNode{arg}, graph::FloatPrec::F64);
            case TokenType::TAN:
                return _graph.addNode(graph::TanNode{arg}, graph::FloatPrec::F64);
            case TokenType::ASIN:
                return _graph.addNode(graph::AsinNode{arg}, graph::FloatPrec::F64);
            case TokenType::ACOS:
                return _graph.addNode(graph::AcosNode{arg}, graph::FloatPrec::F64);
            case TokenType::ATAN:
                return _graph.addNode(graph::AtanNode{arg}, graph::FloatPrec::F64);
            case TokenType::SINH:
                return _graph.addNode(graph::SinhNode{arg}, graph::FloatPrec::F64);
            case TokenType::COSH:
                return _graph.addNode(graph::CoshNode{arg}, graph::FloatPrec::F64);
            case TokenType::TANH:
                return _graph.addNode(graph::TanhNode{arg}, graph::FloatPrec::F64);
            default:
                error("Unknown function");
        }
    }

}  // namespace frontend::satire
