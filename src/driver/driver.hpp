#pragma once

#include "frontend/frontend.hpp"

#include <string>
#include <vector>

namespace driver {
    struct DriverOpts {
        std::string inputFile;
        std::vector<std::string> domainArgs;  // Hybrid: file, or var=[l,u]
        std::optional<std::string> defaultDomain;
        std::string targetFunction;  // LLVM only
        bool emitGraph {false};
        bool emitExpr {false};
        bool verbose {false};
        std::string jsonOutputFile {"results.json"};
        bool jsonToStdout {false};
        bool showAllInstructions {false};
        double timeoutSeconds {30.0};
    };

    bool run(const DriverOpts& opts, const frontend::Frontend& fe);
}  // namespace driver
