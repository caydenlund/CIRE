#pragma once
#include "error_expr/error_expr.hpp"
#include "interval/interval.hpp"

#include <string>
#include <unordered_map>

namespace optimizer {
    struct OptimizerOpts {
        double timeoutSeconds {300.0};
        double relTolerance {1e-6};
        bool verbose {false};
    };

    struct OptimizeResult {
        double upperBound;
        std::unordered_map<std::string, double> witnessInputs;
        bool provedTight {false};
    };

    class Optimizer {
    public:
        virtual ~Optimizer() = default;

        [[nodiscard]] virtual OptimizeResult maximize(const error_expr::ErrorExpr& expr,
                                                      const interval::InputDomain& domain,
                                                      const OptimizerOpts& opts) const
                = 0;
    };
}  // namespace optimizer
