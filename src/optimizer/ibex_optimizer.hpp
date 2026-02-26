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
            const OptimizerOpts& opts) const override;

    private:
        // Convert ErrorExpr to IBEX expression
        struct ConversionContext {
            const error_expr::ErrorExpr& expr;
            const interval::InputDomain& domain;
            std::unordered_map<std::string, const ibex::ExprSymbol*> symbolTable;
            ibex::Array<const ibex::ExprSymbol> symbols;
        };

        const ibex::ExprNode& convertToIbex(
            error_expr::ExprId id,
            ConversionContext& ctx) const;

        void setupSymbols(ConversionContext& ctx) const;
    };
}  // namespace optimizer
