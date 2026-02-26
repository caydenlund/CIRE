#pragma once

#include "graph/computation_graph.hpp"

#include <filesystem>
#include <string>

namespace frontend {
    struct FrontendOpts {
        bool verbose {false};
        bool emitGraph {false};
        std::string targetFunction;  // LLVM only
    };

    class Frontend {
    public:
        virtual ~Frontend() = default;

        [[nodiscard]] virtual graph::ComputationGraph parse(const std::filesystem::path& input_path,
                                                            const FrontendOpts& opts) const
                = 0;
    };
}  // namespace frontend
