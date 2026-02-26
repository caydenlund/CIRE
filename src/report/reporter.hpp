#pragma once
#include "error_expr/error_expr.hpp"
#include "graph/computation_graph.hpp"
#include "optimizer/optimizer.hpp"

#include <ostream>

namespace report {
    class Reporter {
    public:
        explicit Reporter(std::ostream& out) : _out(out) {}

        void print(const graph::ComputationGraph& graph, const error_expr::ErrorExpr& expr,
                   const optimizer::OptimizeResult& result) const;

    private:
        std::ostream& _out;
    };
}  // namespace report
