#pragma once

#include "frontend/frontend.hpp"

#include <optional>
#include <string>
#include <vector>

namespace driver {
    struct DriverOpts {
        std::string inputFile;
        std::vector<std::string> domainArgs;  // Hybrid: file, or var=[l,u]
        std::optional<std::string> defaultDomain;
        std::string targetFunction;  // LLVM only
        std::string outputGraphFile;
        bool emitExpr {false};
        bool verbose {false};
        std::string jsonOutputFile {"results.json"};
        bool jsonToStdout {false};
        bool showAllInstructions {false};
        bool detailed {false};
        double timeoutSeconds {30.0};
    };

    bool run(const DriverOpts& opts, const frontend::Frontend& fe);
}  // namespace driver
