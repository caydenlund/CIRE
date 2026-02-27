#include "interval/interval.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cctype>

namespace interval {

    InputDomain parseDomainFile(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open domain file: " + path);
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string content = buffer.str();

        InputDomain domain;

        // Simple JSON parser for format: { "x": [-1.0, 1.0], "y": [0.0, 3.14] }
        size_t pos = 0;

        // Skip whitespace
        auto skipWhitespace = [&]() {
            while (pos < content.size() && std::isspace(content[pos])) {
                ++pos;
            }
        };

        // Expect a specific character
        auto expect = [&](char ch) {
            skipWhitespace();
            if (pos >= content.size() || content[pos] != ch) {
                throw std::runtime_error(
                    "Parse error in domain file: expected '" + std::string(1, ch) +
                    "' at position " + std::to_string(pos));
            }
            ++pos;
        };

        // Parse a string (variable name)
        auto parseString = [&]() -> std::string {
            skipWhitespace();
            expect('"');
            size_t start = pos;
            while (pos < content.size() && content[pos] != '"') {
                ++pos;
            }
            std::string result = content.substr(start, pos - start);
            expect('"');
            return result;
        };

        // Parse a number
        auto parseNumber = [&]() -> double {
            skipWhitespace();
            size_t start = pos;
            if (pos < content.size() && (content[pos] == '-' || content[pos] == '+')) {
                ++pos;
            }
            while (pos < content.size() &&
                   (std::isdigit(content[pos]) || content[pos] == '.' ||
                    content[pos] == 'e' || content[pos] == 'E' ||
                    content[pos] == '-' || content[pos] == '+')) {
                ++pos;
            }
            std::string numStr = content.substr(start, pos - start);
            try {
                return std::stod(numStr);
            } catch (...) {
                throw std::runtime_error("Invalid number in domain file: " + numStr);
            }
        };

        // Parse an interval [lo, hi]
        auto parseInterval = [&]() -> Interval {
            expect('[');
            double lo = parseNumber();
            expect(',');
            double hi = parseNumber();
            expect(']');
            return Interval{lo, hi};
        };

        // Parse the entire JSON object
        expect('{');

        while (true) {
            skipWhitespace();
            if (pos < content.size() && content[pos] == '}') {
                ++pos;
                break;
            }

            // Parse "varname": [lo, hi]
            std::string varName = parseString();
            expect(':');
            Interval interval = parseInterval();

            domain[varName] = interval;

            // Check for comma or end of object
            skipWhitespace();
            if (pos < content.size() && content[pos] == ',') {
                ++pos;
            } else {
                skipWhitespace();
                if (pos >= content.size() || content[pos] != '}') {
                    throw std::runtime_error(
                        "Parse error: expected ',' or '}' at position " + std::to_string(pos));
                }
            }
        }

        if (domain.empty()) {
            throw std::runtime_error("Domain file contains no variables");
        }

        return domain;
    }

}  // namespace interval
