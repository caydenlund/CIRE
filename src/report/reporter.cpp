#include "reporter.hpp"

#include <cmath>
#include <iomanip>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

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
                        const optimizer::OptimizeResult& result,
                        const std::vector<InstructionErrorInfo>& perInstructionErrors) const {
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

        // Print per-instruction error contributions
        if (!perInstructionErrors.empty()) {
            _out << "\n----------------------------------------\n";
            _out << "Per-Instruction Error Contributions:\n";
            _out << "----------------------------------------\n\n";

            // Print table header
            _out << std::left << std::setw(40) << "Instruction"
                 << std::right << std::setw(18) << "Error Contrib"
                 << std::setw(12) << "Percentage"
                 << "\n";
            _out << std::string(70, '-') << "\n";

            // Print each instruction's error contribution
            for (const auto& inst : perInstructionErrors) {
                _out << std::left << std::setw(40) << inst.irRepresentation
                     << std::scientific << std::setprecision(4) << std::right << std::setw(18)
                     << inst.errorContribution
                     << std::fixed << std::setprecision(2) << std::setw(11)
                     << inst.percentageContribution << "%"
                     << "\n";
            }

            _out << "\nTop contributors shown. Total instructions with error: "
                 << perInstructionErrors.size() << "\n";
        }

        _out << "\n========================================\n";
    }

    void Reporter::printJSON(const JSONReportData& data) const {
        json j;

        // Basic metadata
        j["filename"] = data.filename;
        j["function"] = data.function;

        // Input domains
        json domains;
        for (const auto& [varName, interval] : data.inputDomains) {
            domains[varName] = {interval.lo, interval.hi};
        }
        j["input_domains"] = domains;

        // Results section
        json results;
        results["absolute_error_bound"] = data.result.upperBound;

        // Compute relative error and ULPs if we have the output value
        if (data.result.witnessOutputValue != 0.0) {
            double relativeError = data.result.upperBound / std::abs(data.result.witnessOutputValue);
            double ulp = ulpAt(data.result.witnessOutputValue);
            double ulps = data.result.upperBound / ulp;

            results["relative_error"] = relativeError;
            results["error_in_ulps"] = ulps;
            results["ulp_at_output"] = ulp;
        } else {
            results["relative_error"] = nullptr;
            results["error_in_ulps"] = nullptr;
            results["ulp_at_output"] = nullptr;
        }

        results["status"] = data.result.provedTight ? "proved_tight" : "sound_upper_bound";

        // Witness input
        json witness;
        for (const auto& [name, value] : data.result.witnessInputs) {
            witness[name] = value;
        }
        results["witness_input"] = witness;
        results["output_at_witness"] = data.result.witnessOutputValue;

        // Computation graph info
        json graphInfo;
        graphInfo["nodes"] = data.graph.nodes().size();
        graphInfo["outputs"] = data.graph.outputs().size();
        results["computation_graph"] = graphInfo;

        // Per-instruction errors
        json instructionErrors = json::array();
        for (const auto& instErr : data.perInstructionErrors) {
            json inst;
            inst["node_id"] = instErr.nodeId;
            inst["instruction_name"] = instErr.instructionName;
            inst["instruction_type"] = instErr.instructionType;
            inst["error_contribution"] = instErr.errorContribution;
            inst["percentage_contribution"] = instErr.percentageContribution;
            inst["ir_representation"] = instErr.irRepresentation;

            if (instErr.sourceLocation.has_value()) {
                json loc;
                loc["file"] = instErr.sourceLocation->file;
                loc["line"] = instErr.sourceLocation->line;
                loc["column"] = instErr.sourceLocation->col;
                inst["source_location"] = loc;
            } else {
                inst["source_location"] = nullptr;
            }

            instructionErrors.push_back(inst);
        }
        results["per_instruction_errors"] = instructionErrors;

        j["results"] = results;

        // Write formatted JSON to output
        _out << j.dump(2) << "\n";
    }

}  // namespace report
