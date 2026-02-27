#include "reporter.hpp"

#include <iomanip>

namespace report {

    void Reporter::print(const graph::ComputationGraph& graph,
                        const error_expr::ErrorExpr& expr,
                        const optimizer::OptimizeResult& result) const {
        _out << "========================================\n";
        _out << "CIRE Error Analysis Report\n";
        _out << "========================================\n\n";

        // Print graph information
        _out << "Computation Graph:\n";
        _out << "  Nodes: " << graph.nodes().size() << "\n";
        _out << "  Outputs: " << graph.outputs().size() << "\n\n";

        // Print error bound
        _out << "Error Analysis Results:\n";
        _out << "  Upper Bound: " << std::scientific << std::setprecision(10)
             << result.upperBound << "\n";

        if (result.provedTight) {
            _out << "  Status: Proved tight\n";
        } else {
            _out << "  Status: Sound upper bound (not necessarily tight)\n";
        }

        // Print witness inputs if available
        if (!result.witnessInputs.empty()) {
            _out << "\n  Witness Input (achieving worst case):\n";
            for (const auto& [name, value] : result.witnessInputs) {
                _out << "    " << name << " = " << std::scientific << std::setprecision(10)
                     << value << "\n";
            }
        }

        _out << "\n========================================\n";
    }

}  // namespace report
