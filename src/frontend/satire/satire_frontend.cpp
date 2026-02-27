#include "satire_frontend.hpp"
#include "parser.hpp"
#include "lexer.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace frontend::satire {

    graph::ComputationGraph SatireFrontend::parse(const std::filesystem::path& input_path,
                                                   const FrontendOpts& opts) const {
        if (!std::filesystem::exists(input_path)) {
            throw std::runtime_error("Input file does not exist: " + input_path.string());
        }

        // Read file
        std::ifstream file(input_path);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open input file: " + input_path.string());
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string source = buffer.str();

        if (opts.verbose) {
            std::cout << "Parsing SATIRE file: " << input_path << std::endl;
        }

        // Lexer and parser
        Lexer lexer(source);
        Parser parser(lexer);

        try {
            graph::ComputationGraph g = parser.parse();

            if (opts.verbose) {
                std::cout << "Successfully parsed graph with " << g.nodes().size()
                         << " nodes and " << g.outputs().size() << " outputs" << std::endl;
            }

            return g;
        } catch (const std::exception& e) {
            throw std::runtime_error("Failed to parse SATIRE file: " + std::string(e.what()));
        }
    }

}  // namespace frontend::satire
