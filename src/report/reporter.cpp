#include "reporter.hpp"

#include <cmath>
#include <iomanip>
#include <nlohmann/json.hpp>
#include <sstream>
#include <unordered_map>

using json = nlohmann::json;

namespace report {
    namespace {
        std::string floatPrecToString(graph::FloatPrec prec) {
            switch (prec) {
                case graph::FloatPrec::F16: return "f16";
                case graph::FloatPrec::BF16: return "bf16";
                case graph::FloatPrec::F32: return "f32";
                case graph::FloatPrec::F64: return "f64";
                case graph::FloatPrec::F128: return "f128";
            }
            return "unknown";
        }

        std::string formatConstant(double value) {
            std::ostringstream oss;
            oss << std::setprecision(17) << value;
            return oss.str();
        }

        std::string parenthesizeBinary(const std::string& lhs, const std::string& op, const std::string& rhs) {
            return "(" + lhs + " " + op + " " + rhs + ")";
        }

        std::string formatFunctionCall(const std::string& name, const std::string& arg) {
            return name + "(" + arg + ")";
        }

        std::string formatComputationExpression(
                graph::NodeId id,
                const graph::ComputationGraph& graph,
                std::unordered_map<graph::NodeId, std::string>& cache) {
            auto cached = cache.find(id);
            if (cached != cache.end()) {
                return cached->second;
            }

            const graph::Node& node = graph.getNode(id);
            std::string expression = std::visit(
                graph::Overloaded{
                    [](const graph::InputVarNode& n) {
                        return n.name;
                    },
                    [](const graph::ConstantNode& n) {
                        return formatConstant(n.value);
                    },
                    [&](const graph::AddNode& n) {
                        return parenthesizeBinary(formatComputationExpression(n.lhs, graph, cache), "+",
                                                  formatComputationExpression(n.rhs, graph, cache));
                    },
                    [&](const graph::SubNode& n) {
                        return parenthesizeBinary(formatComputationExpression(n.lhs, graph, cache), "-",
                                                  formatComputationExpression(n.rhs, graph, cache));
                    },
                    [&](const graph::MulNode& n) {
                        return parenthesizeBinary(formatComputationExpression(n.lhs, graph, cache), "*",
                                                  formatComputationExpression(n.rhs, graph, cache));
                    },
                    [&](const graph::DivNode& n) {
                        return parenthesizeBinary(formatComputationExpression(n.lhs, graph, cache), "/",
                                                  formatComputationExpression(n.rhs, graph, cache));
                    },
                    [&](const graph::PowNode& n) {
                        return "pow(" + formatComputationExpression(n.lhs, graph, cache) + ", " +
                               formatComputationExpression(n.rhs, graph, cache) + ")";
                    },
                    [&](const graph::NegNode& n) {
                        return "(-" + formatComputationExpression(n.src, graph, cache) + ")";
                    },
                    [&](const graph::SqrtNode& n) {
                        return formatFunctionCall("sqrt", formatComputationExpression(n.src, graph, cache));
                    },
                    [&](const graph::AbsNode& n) {
                        return formatFunctionCall("abs", formatComputationExpression(n.src, graph, cache));
                    },
                    [&](const graph::SinNode& n) {
                        return formatFunctionCall("sin", formatComputationExpression(n.src, graph, cache));
                    },
                    [&](const graph::CosNode& n) {
                        return formatFunctionCall("cos", formatComputationExpression(n.src, graph, cache));
                    },
                    [&](const graph::TanNode& n) {
                        return formatFunctionCall("tan", formatComputationExpression(n.src, graph, cache));
                    },
                    [&](const graph::AsinNode& n) {
                        return formatFunctionCall("asin", formatComputationExpression(n.src, graph, cache));
                    },
                    [&](const graph::AcosNode& n) {
                        return formatFunctionCall("acos", formatComputationExpression(n.src, graph, cache));
                    },
                    [&](const graph::AtanNode& n) {
                        return formatFunctionCall("atan", formatComputationExpression(n.src, graph, cache));
                    },
                    [&](const graph::SinhNode& n) {
                        return formatFunctionCall("sinh", formatComputationExpression(n.src, graph, cache));
                    },
                    [&](const graph::CoshNode& n) {
                        return formatFunctionCall("cosh", formatComputationExpression(n.src, graph, cache));
                    },
                    [&](const graph::TanhNode& n) {
                        return formatFunctionCall("tanh", formatComputationExpression(n.src, graph, cache));
                    },
                    [&](const graph::ExpNode& n) {
                        return formatFunctionCall("exp", formatComputationExpression(n.src, graph, cache));
                    },
                    [&](const graph::LogNode& n) {
                        return formatFunctionCall("log", formatComputationExpression(n.src, graph, cache));
                    },
                    [&](const graph::CastNode& n) {
                        return "cast_" + floatPrecToString(n.from) + "_to_" + floatPrecToString(n.to) + "(" +
                               formatComputationExpression(n.src, graph, cache) + ")";
                    },
                    [&](const graph::FmaNode& n) {
                        return "fma(" + formatComputationExpression(n.a, graph, cache) + ", " +
                               formatComputationExpression(n.b, graph, cache) + ", " +
                               formatComputationExpression(n.c, graph, cache) + ")";
                    },
                    [&](const graph::ReduceSumNode& n) {
                        std::string expr = "reduce_sum(" + formatComputationExpression(n.src, graph, cache);
                        if (n.axis.has_value()) {
                            expr += ", axis=" + std::to_string(*n.axis);
                        }
                        expr += ")";
                        return expr;
                    },
                },
                node.kind);

            cache[id] = expression;
            return expression;
        }

        std::string formatComputationExpression(const graph::ComputationGraph& graph) {
            if (graph.outputs().empty()) {
                return "";
            }

            std::unordered_map<graph::NodeId, std::string> cache;

            if (graph.outputs().size() == 1) {
                return formatComputationExpression(graph.outputs().front(), graph, cache);
            }

            std::ostringstream oss;
            oss << "(";
            for (std::size_t i = 0; i < graph.outputs().size(); ++i) {
                if (i > 0) {
                    oss << ", ";
                }
                oss << formatComputationExpression(graph.outputs()[i], graph, cache);
            }
            oss << ")";
            return oss.str();
        }
    }

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
                        const std::vector<InstructionErrorInfo>& perInstructionErrors,
                        bool detailed) const {
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
        // TODO: Handle this
        // if (result.witnessOutputValue != 0.0) {
        //     double relativeError = result.upperBound / std::abs(result.witnessOutputValue);
        //     double ulp = ulpAt(result.witnessOutputValue);
        //     double ulps = result.upperBound / ulp;

        //     _out << "  Relative Error: " << std::scientific << std::setprecision(3)
        //          << relativeError << "\n";
        //     _out << "  Error in ULPs: " << std::fixed << std::setprecision(2)
        //          << ulps << "\n";
        //     _out << "  (1 ULP at output ≈ " << std::scientific << std::setprecision(3)
        //          << ulp << ")\n";
        //     _out << "  NOTE: Relative + ULPs are based purely on witness output\n";
        // }

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

        if (detailed) {
            printComputationExpression(graph);
            printOptimizerDetails(result);
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

    void Reporter::printComputationExpression(const graph::ComputationGraph& graph) const {
        _out << "\nComputation Expression:\n";
        const std::string expression = formatComputationExpression(graph);
        _out << "  " << (expression.empty() ? "<none>" : expression) << "\n";
    }

    void Reporter::printOptimizerDetails(const optimizer::OptimizeResult& result) const {
        _out << "\nIBEX Optimization Details:\n";
        _out << "  Goal expression:\n";
        _out << "    "
             << (result.optimizationExpression.empty() ? "<unavailable>" : result.optimizationExpression)
             << "\n";
        _out << "  Options:\n";
        for (const auto& option : result.optimizerOptions) {
            _out << "    " << option.name << " = " << option.value << "\n";
        }
        if (result.optimizerOptions.empty()) {
            _out << "    <none recorded>\n";
        }
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
        // TODO: Handle this
        // if (data.result.witnessOutputValue != 0.0) {
        //     double relativeError = data.result.upperBound / std::abs(data.result.witnessOutputValue);
        //     double ulp = ulpAt(data.result.witnessOutputValue);
        //     double ulps = data.result.upperBound / ulp;

        //     results["relative_error"] = relativeError;
        //     results["error_in_ulps"] = ulps;
        //     results["ulp_at_output"] = ulp;
        // } else {
        //     results["relative_error"] = nullptr;
        //     results["error_in_ulps"] = nullptr;
        //     results["ulp_at_output"] = nullptr;
        // }

        results["status"] = data.result.provedTight ? "proved_tight" : "sound_upper_bound";

        results["computation_expression"] = formatComputationExpression(data.graph);

        json optimizerInfo;
        optimizerInfo["name"] = data.result.optimizerName;
        optimizerInfo["optimization_expression"] = data.result.optimizationExpression;

        json optimizerOptions = json::array();
        for (const auto& option : data.result.optimizerOptions) {
            json optionInfo;
            optionInfo["name"] = option.name;
            optionInfo["value"] = option.value;
            optimizerOptions.push_back(optionInfo);
        }
        optimizerInfo["options"] = optimizerOptions;
        results["optimizer"] = optimizerInfo;

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
