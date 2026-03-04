#include "reporter.hpp"

#include <cmath>
#include <iomanip>

namespace report {

    // Helper function to compute ULP at a given magnitude
    static double ulpAt(double value) {
        if (value == 0.0) {
            return 5e-324;  // Smallest positive double
        }

        double absValue = std::abs(value);
        // ULP = 2^(exponent - 52) for double precision
        int exponent;
        std::frexp(absValue, &exponent);  // Get exponent
        return std::ldexp(1.0, exponent - 53);  // 2^(exponent - 53)
    }

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
        _out << "  Absolute Error Bound: " << std::scientific << std::setprecision(10)
             << result.upperBound << "\n";

        // Compute and print relative error and ULPs if we have the output value
        if (result.witnessOutputValue != 0.0) {
            double relativeError = result.upperBound / std::abs(result.witnessOutputValue);
            double ulp = ulpAt(result.witnessOutputValue);
            double ulps = result.upperBound / ulp;

            _out << "  Relative Error: " << std::scientific << std::setprecision(3)
                 << relativeError << "\n";
            _out << "  Error in ULPs: " << std::fixed << std::setprecision(2)
                 << ulps << "\n";
            _out << "  (1 ULP at output ≈ " << std::scientific << std::setprecision(3)
                 << ulp << ")\n";
        }

        if (result.provedTight) {
            _out << "\n  Status: Proved tight\n";
        } else {
            _out << "\n  Status: Sound upper bound (not necessarily tight)\n";
        }

        // Print witness inputs if available
        if (!result.witnessInputs.empty()) {
            _out << "\n  Witness Input (achieving worst case):\n";
            for (const auto& [name, value] : result.witnessInputs) {
                _out << "    " << name << " = " << std::scientific << std::setprecision(10)
                     << value << "\n";
            }

            if (result.witnessOutputValue != 0.0) {
                _out << "\n  Output at witness: " << std::scientific << std::setprecision(10)
                     << result.witnessOutputValue << "\n";
            }
        }

        _out << "\n========================================\n";
    }

}  // namespace report
