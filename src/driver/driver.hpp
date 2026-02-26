#pragma once

#include "frontend/frontend.hpp"

#include <string>

namespace driver {
    struct DriverOpts {
        std::string inputFile;
        std::string domainFile;
        std::string targetFunction;  // LLVM only
        bool emitGraph {false};
        bool emitExpr {false};
        bool verbose {false};
    };

    bool run(const DriverOpts& opts, const frontend::Frontend& fe);
}  // namespace driver
