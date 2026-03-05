#pragma once

#include <optional>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

namespace interval {
    struct Interval {
        double lo, hi;

        static Interval point(double v) { return {v, v}; }
        bool contains(double v) const { return v >= lo && v <= hi; }
        double width() const { return hi - lo; }
        bool isDegenerate() const { return lo > hi; }
    };

    using InputDomain = std::unordered_map<std::string, Interval>;

    /// Parse a domain specification from a JSON file.
    /// Expected schema: { "x": [-1.0, 1.0], "y": [0.0, 3.14] }
    [[nodiscard]] InputDomain parseDomainFile(const std::string& path);

    /// Parse domain arguments:
    /// - "--domain <var>=[l,u]" - domain for specific variable
    /// - "--domain <filename>" - JSON domain file
    /// - "--default-domain [l,u]" - default domain for all variables
    [[nodiscard]] InputDomain parseDomainArgs(const std::vector<std::string>& args,
                                              const std::optional<std::string>& defaultInterval,
                                              const std::vector<std::string>& inputVarNames);
}  // namespace interval
