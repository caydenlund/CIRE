#include "interval/interval.hpp"

#include <cctype>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <optional>
#include <regex>
#include <sstream>
#include <stdexcept>

namespace interval {

    // Helper: Parse interval from string "[l,u]"
    static Interval parseIntervalString(const std::string& str) {
        std::regex intervalRegex(
                R"(\[\s*([-+]?[0-9]*\.?[0-9]+(?:[eE][-+]?[0-9]+)?)\s*,\s*([-+]?[0-9]*\.?[0-9]+(?:[eE][-+]?[0-9]+)?)\s*\])");
        std::smatch match;

        if (!std::regex_match(str, match, intervalRegex)) {
            throw std::runtime_error("Invalid interval format: " + str + " (expected [l,u])");
        }

        try {
            double lo = std::stod(match[1].str());
            double hi = std::stod(match[2].str());

            if (lo > hi) { throw std::runtime_error("Invalid interval: lower bound > upper bound in " + str); }

            return Interval {lo, hi};
        } catch (const std::invalid_argument& e) {
            throw std::runtime_error("Failed to parse numbers in interval: " + str);
        }
    }

    InputDomain parseDomainFile(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) { throw std::runtime_error("Failed to open domain file: " + path); }

        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string content = buffer.str();

        InputDomain domain;

        // Simple JSON parser for format: { "x": [-1.0, 1.0], "y": [0.0, 3.14] }
        size_t pos = 0;

        // Skip whitespace
        auto skipWhitespace = [&]() {
            while (pos < content.size() && std::isspace(content[pos])) { ++pos; }
        };

        // Expect a specific character
        auto expect = [&](char ch) {
            skipWhitespace();
            if (pos >= content.size() || content[pos] != ch) {
                throw std::runtime_error("Parse error in domain file: expected '" + std::string(1, ch)
                                         + "' at position " + std::to_string(pos));
            }
            ++pos;
        };

        // Parse a string (variable name)
        auto parseString = [&]() -> std::string {
            skipWhitespace();
            expect('"');
            size_t start = pos;
            while (pos < content.size() && content[pos] != '"') { ++pos; }
            std::string result = content.substr(start, pos - start);
            expect('"');
            return result;
        };

        // Parse a number
        auto parseNumber = [&]() -> double {
            skipWhitespace();
            size_t start = pos;
            if (pos < content.size() && (content[pos] == '-' || content[pos] == '+')) { ++pos; }
            while (pos < content.size()
                   && (std::isdigit(content[pos]) || content[pos] == '.' || content[pos] == 'e' || content[pos] == 'E'
                       || content[pos] == '-' || content[pos] == '+')) {
                ++pos;
            }
            std::string numStr = content.substr(start, pos - start);
            try {
                return std::stod(numStr);
            } catch (...) { throw std::runtime_error("Invalid number in domain file: " + numStr); }
        };

        // Parse an interval [lo, hi]
        auto parseInterval = [&]() -> Interval {
            expect('[');
            double lo = parseNumber();
            expect(',');
            double hi = parseNumber();
            expect(']');
            return Interval {lo, hi};
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
                    throw std::runtime_error("Parse error: expected ',' or '}' at position " + std::to_string(pos));
                }
            }
        }

        if (domain.empty()) { throw std::runtime_error("Domain file contains no variables"); }

        return domain;
    }

    InputDomain parseDomainArgs(const std::vector<std::string>& args, const std::optional<std::string>& defaultInterval,
                                const std::vector<std::string>& inputVarNames) {
        if (args.empty() && !defaultInterval.has_value()) {
            throw std::runtime_error("No domain arguments provided (use -d or --domain)");
        }
        InputDomain domain;

        // Process arguments in order to establish priority
        for (const auto& arg : args) {
            // Check if it's a variable assignment: var=[l,u]
            size_t eqPos = arg.find('=');
            if (eqPos != std::string::npos && eqPos > 0 && arg[eqPos + 1] == '[') {
                // Variable-specific domain: var=[l,u]
                std::string varName = arg.substr(0, eqPos);
                std::string intervalStr = arg.substr(eqPos + 1);

                Interval interval = parseIntervalString(intervalStr);
                domain[varName] = interval;
            } else {
                // Assume it's a filename
                if (!std::filesystem::exists(arg)) { throw std::runtime_error("Domain file not found: " + arg); }

                InputDomain fileDomain = parseDomainFile(arg);

                // Merge file domain (doesn't override existing variable-specific domains)
                for (const auto& [varName, interval] : fileDomain) {
                    if (domain.find(varName) == domain.end()) { domain[varName] = interval; }
                }
            }
        }

        // Apply default interval to any missing variables
        if (defaultInterval.has_value()) {
            const Interval interval = parseIntervalString(defaultInterval.value());
            for (const auto& varName : inputVarNames) {
                if (domain.find(varName) == domain.end()) domain[varName] = interval;
            }
        }

        // Validate that all input variables have domains
        for (const auto& varName : inputVarNames) {
            if (domain.find(varName) == domain.end()) {
                throw std::runtime_error("No domain specified for input variable '" + varName + "'. "
                                         + "Provide a default domain with -d [l,u], a domain file, or "
                                         + "a variable-specific domain with -d " + varName + "=[l,u]");
            }
        }

        return domain;
    }

}  // namespace interval
