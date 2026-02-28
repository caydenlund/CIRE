#pragma once

#include "optimizer.hpp"
#include "error_expr/error_expr.hpp"
#include "interval/interval.hpp"

#include <ibex.h>
#include <memory>
#include <unordered_map>

namespace optimizer {
    /**
     * IBEX-based global optimizer for error expressions.
     * Uses interval constraint propagation and branch-and-bound.
     */
    class IbexOptimizer : public Optimizer {
    public:
        IbexOptimizer() = default;
        ~IbexOptimizer() override = default;

        [[nodiscard]] OptimizeResult maximize(
            const error_expr::ErrorExpr& expr,
            const interval::InputDomain& domain,
            const graph::ComputationGraph& graph,
            const std::unordered_map<graph::NodeId, error_expr::ExprId>& symbolicVal,
            const OptimizerOpts& opts) const override;

    private:
        // Convert ErrorExpr to IBEX expression
        struct ConversionContext {
            const error_expr::ErrorExpr& expr;
            const interval::InputDomain& domain;
            const graph::ComputationGraph& graph;
            const std::unordered_map<graph::NodeId, error_expr::ExprId>& symbolicVal;
            std::unordered_map<std::string, const ibex::ExprSymbol*> symbolTable;
            mutable std::unordered_map<graph::NodeId, const ibex::ExprNode*> nodeCache;
            ibex::Array<const ibex::ExprSymbol> symbols;
        };

        const ibex::ExprNode& convertToIbex(
            error_expr::ExprId id,
            ConversionContext& ctx) const;

        const ibex::ExprNode& convertNodeToIbex(
            graph::NodeId nodeId,
            ConversionContext& ctx) const;

        void setupSymbols(ConversionContext& ctx) const;
    };
}  // namespace optimizer
