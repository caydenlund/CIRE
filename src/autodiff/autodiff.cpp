#include "autodiff.hpp"
#include "graph/node.hpp"

// rules.cpp is in the same translation unit group;
// applyRule is declared here and defined in rules.cpp
namespace autodiff::detail {
    std::pair<error_expr::ExprId, error_expr::ExprId>
    applyRule(const graph::Node& node, error_expr::ErrorExpr& arena,
              const std::unordered_map<graph::NodeId, error_expr::ExprId>& val,
              const std::unordered_map<graph::NodeId, error_expr::ExprId>& err);
}  // namespace autodiff::detail

namespace autodiff {
    AutodiffResult analyze(const graph::ComputationGraph& g) {
        AutodiffResult result;
        auto& arena = result.expr;
        auto& val = result.symbolicVal;
        auto& err = result.symbolicErr;

        for (graph::NodeId id : g.topoOrder()) {
            const graph::Node& node = g.getNode(id);
            auto [v, e] = detail::applyRule(node, arena, val, err);
            val[id] = v;
            err[id] = e;
        }

        // The root error expression is the sum of errors over all outputs
        if (g.outputs().size() == 1) {
            arena.setRoot(err.at(g.outputs().front()));
        } else {
            error_expr::ExprId combined = arena.makeConst(0.0);
            for (graph::NodeId out : g.outputs()) combined = arena.makeAdd(combined, err.at(out));
            arena.setRoot(combined);
        }

        return result;
    }
}  // namespace autodiff
