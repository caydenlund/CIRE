#pragma once

#include "graph/computation_graph.hpp"
#include "lexer.hpp"

#include <unordered_map>
#include <string>

namespace frontend::satire {

    class Parser {
    public:
        explicit Parser(Lexer& lexer);

        graph::ComputationGraph parse();

    private:
        Lexer& _lexer;
        Token _current;
        graph::ComputationGraph _graph;
        std::unordered_map<std::string, graph::NodeId> _symbolTable;
        std::vector<std::string> _outputNames;

        // Token manipulation
        void advance();
        bool check(TokenType type);
        bool match(TokenType type);
        void consume(TokenType type, const std::string& message);

        // Parsing sections
        void parseInputs();
        void parseOutputs();
        void parseExprs();

        // Parsing expressions
        graph::NodeId parseExpression();
        graph::NodeId parseAdditive();
        graph::NodeId parseMultiplicative();
        graph::NodeId parseUnary();
        graph::NodeId parsePrimary();
        graph::NodeId parseFunction(TokenType funcType);

        // Type conversion
        graph::FloatPrec tokenToPrec(TokenType type);

        // Error reporting
        [[noreturn]] void error(const std::string& message);
    };

}  // namespace frontend::satire
