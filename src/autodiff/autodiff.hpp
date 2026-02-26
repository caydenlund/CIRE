#pragma once

#include "error_expr/error_expr.hpp"
#include "graph/computation_graph.hpp"

#include <unordered_map>

namespace autodiff {
    struct AutodiffResult {
        error_expr::ErrorExpr expr;
        std::unordered_map<graph::NodeId, error_expr::ExprId> symbolicVal;
        std::unordered_map<graph::NodeId, error_expr::ExprId> symbolicErr;
    };

    /// Walk the graph in topological order, applying per-op error rules.
    [[nodiscard]] AutodiffResult analyze(const graph::ComputationGraph& g);
}  // namespace autodiff
