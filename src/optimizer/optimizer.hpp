#pragma once
#include "error_expr/error_expr.hpp"
#include "graph/computation_graph.hpp"
#include "interval/interval.hpp"

#include <string>
#include <unordered_map>
#include <vector>

namespace optimizer {
    struct OptimizerOpts {
        double timeoutSeconds {30.0};
        double relTolerance {1e-6};
        bool verbose {false};
    };

    struct OptimizerOption {
        std::string name;
        std::string value;
    };

    struct OptimizeResult {
        double upperBound;
        std::unordered_map<std::string, double> witnessInputs;
        bool provedTight {false};
        double witnessOutputValue {0.0};  // Value of output at witness point
        std::string optimizerName;
        std::string optimizationExpression;
        std::vector<OptimizerOption> optimizerOptions;
    };

    class Optimizer {
    public:
        virtual ~Optimizer() = default;

        [[nodiscard]] virtual OptimizeResult maximize(
                const error_expr::ErrorExpr& expr,
                const interval::InputDomain& domain,
                const graph::ComputationGraph& graph,
                const std::unordered_map<graph::NodeId, error_expr::ExprId>& symbolicVal,
                const OptimizerOpts& opts) const
                = 0;
    };
}  // namespace optimizer
