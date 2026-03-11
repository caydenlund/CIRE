#include "driver.hpp"
#include "autodiff/autodiff.hpp"
#include "frontend/frontend.hpp"
#include "interval/interval.hpp"
#include "optimizer/ibex_optimizer.hpp"
#include "report/reporter.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <unordered_map>

namespace driver {
    // Helper: Evaluate an error expression at a given point
    static double evaluateErrorExpr(
            const error_expr::ErrorExpr& expr,
            error_expr::ExprId exprId,
            const std::unordered_map<std::string, double>& varValues) {

        const error_expr::ExprKind& kind = expr.get(exprId);

        return std::visit(
            [&](const auto& node) -> double {
                using T = std::decay_t<decltype(node)>;

                if constexpr (std::is_same_v<T, error_expr::EVarExpr>) {
                    auto it = varValues.find(node.name);
                    if (it != varValues.end()) {
                        return it->second;
                    }
                    return 0.0;  // Unknown variable
                }
                else if constexpr (std::is_same_v<T, error_expr::EErrVar>) {
                    // Look up error variable value from context (for per-node analysis)
                    auto it = varValues.find(node.name);
                    if (it != varValues.end()) {
                        return it->second;
                    }
                    return 1.0;  // Default: worst case
                }
                else if constexpr (std::is_same_v<T, error_expr::EConst>) {
                    return node.value;
                }
                else if constexpr (std::is_same_v<T, error_expr::EEpsilon>) {
                    return graph::unitRoundoff(node.prec);
                }
                else if constexpr (std::is_same_v<T, error_expr::EAdd>) {
                    return evaluateErrorExpr(expr, node.lhs, varValues) +
                           evaluateErrorExpr(expr, node.rhs, varValues);
                }
                else if constexpr (std::is_same_v<T, error_expr::ESub>) {
                    return evaluateErrorExpr(expr, node.lhs, varValues) -
                           evaluateErrorExpr(expr, node.rhs, varValues);
                }
                else if constexpr (std::is_same_v<T, error_expr::EMul>) {
                    return evaluateErrorExpr(expr, node.lhs, varValues) *
                           evaluateErrorExpr(expr, node.rhs, varValues);
                }
                else if constexpr (std::is_same_v<T, error_expr::EDiv>) {
                    double denom = evaluateErrorExpr(expr, node.rhs, varValues);
                    if (denom == 0.0) return 0.0;
                    return evaluateErrorExpr(expr, node.lhs, varValues) / denom;
                }
                else if constexpr (std::is_same_v<T, error_expr::ENeg>) {
                    return -evaluateErrorExpr(expr, node.src, varValues);
                }
                else if constexpr (std::is_same_v<T, error_expr::EAbs>) {
                    return std::abs(evaluateErrorExpr(expr, node.src, varValues));
                }
                else if constexpr (std::is_same_v<T, error_expr::EPow>) {
                    double base = evaluateErrorExpr(expr, node.base, varValues);
                    return std::pow(base, node.exp);
                }
                else {
                    return 0.0;
                }
            },
            kind);
    }

    // Helper: Convert node kind to operation type string
    static std::string nodeKindToString(const graph::NodeKind& kind) {
        return std::visit(
            [](const auto& node) -> std::string {
                using T = std::decay_t<decltype(node)>;
                if constexpr (std::is_same_v<T, graph::InputVarNode>) return "input";
                else if constexpr (std::is_same_v<T, graph::ConstantNode>) return "const";
                else if constexpr (std::is_same_v<T, graph::AddNode>) return "fadd";
                else if constexpr (std::is_same_v<T, graph::SubNode>) return "fsub";
                else if constexpr (std::is_same_v<T, graph::MulNode>) return "fmul";
                else if constexpr (std::is_same_v<T, graph::DivNode>) return "fdiv";
                else if constexpr (std::is_same_v<T, graph::PowNode>) return "pow";
                else if constexpr (std::is_same_v<T, graph::NegNode>) return "fneg";
                else if constexpr (std::is_same_v<T, graph::SqrtNode>) return "sqrt";
                else if constexpr (std::is_same_v<T, graph::AbsNode>) return "fabs";
                else if constexpr (std::is_same_v<T, graph::SinNode>) return "sin";
                else if constexpr (std::is_same_v<T, graph::CosNode>) return "cos";
                else if constexpr (std::is_same_v<T, graph::TanNode>) return "tan";
                else if constexpr (std::is_same_v<T, graph::AsinNode>) return "asin";
                else if constexpr (std::is_same_v<T, graph::AcosNode>) return "acos";
                else if constexpr (std::is_same_v<T, graph::AtanNode>) return "atan";
                else if constexpr (std::is_same_v<T, graph::SinhNode>) return "sinh";
                else if constexpr (std::is_same_v<T, graph::CoshNode>) return "cosh";
                else if constexpr (std::is_same_v<T, graph::TanhNode>) return "tanh";
                else if constexpr (std::is_same_v<T, graph::ExpNode>) return "exp";
                else if constexpr (std::is_same_v<T, graph::LogNode>) return "log";
                else if constexpr (std::is_same_v<T, graph::CastNode>) return "fpcast";
                else if constexpr (std::is_same_v<T, graph::FmaNode>) return "fma";
                else if constexpr (std::is_same_v<T, graph::ReduceSumNode>) return "reduce_sum";
                else return "unknown";
            },
            kind);
    }

    // Helper: Generate IR-like representation for a node
    static std::string generateIRRepresentation(const graph::Node& node) {
        std::ostringstream oss;
        oss << "%" << node.id << " = " << nodeKindToString(node.kind);

        // Add operand information
        std::visit(
            [&](const auto& n) {
                using T = std::decay_t<decltype(n)>;
                if constexpr (std::is_same_v<T, graph::InputVarNode>) {
                    oss << " " << n.name;
                }
                else if constexpr (std::is_same_v<T, graph::ConstantNode>) {
                    oss << " " << n.value;
                }
                else if constexpr (std::is_same_v<T, graph::AddNode> ||
                                   std::is_same_v<T, graph::SubNode> ||
                                   std::is_same_v<T, graph::MulNode> ||
                                   std::is_same_v<T, graph::DivNode> ||
                                   std::is_same_v<T, graph::PowNode>) {
                    oss << " %" << n.lhs << ", %" << n.rhs;
                }
                else if constexpr (std::is_same_v<T, graph::NegNode> ||
                                   std::is_same_v<T, graph::SqrtNode> ||
                                   std::is_same_v<T, graph::AbsNode> ||
                                   std::is_same_v<T, graph::SinNode> ||
                                   std::is_same_v<T, graph::CosNode> ||
                                   std::is_same_v<T, graph::TanNode> ||
                                   std::is_same_v<T, graph::ExpNode> ||
                                   std::is_same_v<T, graph::LogNode>) {
                    oss << " %" << n.src;
                }
                else if constexpr (std::is_same_v<T, graph::FmaNode>) {
                    oss << " %" << n.a << ", %" << n.b << ", %" << n.c;
                }
            },
            node.kind);

        return oss.str();
    }

    bool run(const DriverOpts& opts, const frontend::Frontend& fe) {
        // 1. Parse → ComputationGraph
        frontend::FrontendOpts fopts {
                .verbose = opts.verbose,
                .emitGraph = opts.emitGraph,
                .targetFunction = opts.targetFunction,
        };
        graph::ComputationGraph g = fe.parse(opts.inputFile, fopts);
        g.validate();

        if (opts.emitGraph) g.dumpDot(std::cout);

        // 2. Load input domain (json file or var=[l,u])
        std::vector<std::string> inputVarNames;
        for (const auto& [name, _] : g.inputs()) inputVarNames.push_back(name);
        interval::InputDomain domain = interval::parseDomainArgs(opts.domainArgs, opts.defaultDomain, inputVarNames);

        // 3. Symbolic autodiff → error expression
        autodiff::AutodiffResult ad = autodiff::analyze(g);

        if (opts.emitExpr) ad.expr.dumpAST(std::cout);

        // 4. Select optimizer
        std::unique_ptr<optimizer::Optimizer> opt = std::make_unique<optimizer::IbexOptimizer>();

        optimizer::OptimizerOpts oopts {.verbose = opts.verbose};
        optimizer::OptimizeResult result = opt->maximize(ad.expr, domain, g, ad.symbolicVal, oopts);

        // 5. Compute per-node error contributions
        std::vector<report::InstructionErrorInfo> perInstructionErrors;

        if (!result.witnessInputs.empty()) {
            // Build evaluation context with witness inputs
            std::unordered_map<std::string, double> evalContext = result.witnessInputs;

            // First, evaluate all node values at the witness point
            std::unordered_map<graph::NodeId, double> nodeValues;
            for (graph::NodeId id : g.topoOrder()) {
                const graph::Node& node = g.getNode(id);

                double value = std::visit(
                    [&](const auto& n) -> double {
                        using T = std::decay_t<decltype(n)>;
                        if constexpr (std::is_same_v<T, graph::InputVarNode>) {
                            auto it = evalContext.find(n.name);
                            return (it != evalContext.end()) ? it->second : 0.0;
                        }
                        else if constexpr (std::is_same_v<T, graph::ConstantNode>) {
                            return n.value;
                        }
                        else if constexpr (std::is_same_v<T, graph::AddNode>) {
                            return nodeValues[n.lhs] + nodeValues[n.rhs];
                        }
                        else if constexpr (std::is_same_v<T, graph::SubNode>) {
                            return nodeValues[n.lhs] - nodeValues[n.rhs];
                        }
                        else if constexpr (std::is_same_v<T, graph::MulNode>) {
                            return nodeValues[n.lhs] * nodeValues[n.rhs];
                        }
                        else if constexpr (std::is_same_v<T, graph::DivNode>) {
                            return nodeValues[n.lhs] / nodeValues[n.rhs];
                        }
                        else if constexpr (std::is_same_v<T, graph::NegNode>) {
                            return -nodeValues[n.src];
                        }
                        else if constexpr (std::is_same_v<T, graph::SqrtNode>) {
                            return std::sqrt(nodeValues[n.src]);
                        }
                        else if constexpr (std::is_same_v<T, graph::SinNode>) {
                            return std::sin(nodeValues[n.src]);
                        }
                        else if constexpr (std::is_same_v<T, graph::CosNode>) {
                            return std::cos(nodeValues[n.src]);
                        }
                        else if constexpr (std::is_same_v<T, graph::ExpNode>) {
                            return std::exp(nodeValues[n.src]);
                        }
                        else if constexpr (std::is_same_v<T, graph::LogNode>) {
                            return std::log(nodeValues[n.src]);
                        }
                        else if constexpr (std::is_same_v<T, graph::FmaNode>) {
                            return nodeValues[n.a] * nodeValues[n.b] + nodeValues[n.c];
                        }
                        else {
                            return 0.0;
                        }
                    },
                    node.kind);

                nodeValues[id] = value;
            }

            // Compute total error for percentage calculation
            double totalError = result.upperBound;

            // Compute error contribution for each node by evaluating its error symbol's coefficient
            for (const graph::Node& node : g.nodes()) {
                // Skip input nodes and constants (they don't contribute rounding error)
                bool isLeaf = std::visit(
                    [](const auto& n) -> bool {
                        using T = std::decay_t<decltype(n)>;
                        return std::is_same_v<T, graph::InputVarNode> ||
                               std::is_same_v<T, graph::ConstantNode>;
                    },
                    node.kind);

                if (isLeaf) {
                    continue;  // Inputs and constants don't introduce rounding error
                }

                // To find this node's contribution to the final error:
                // Set eps_N = 1 and all other eps = 0, then evaluate the error expression
                std::unordered_map<std::string, double> epsContext = evalContext;

                // Set all error symbols to 0
                for (const auto& errVar : ad.expr.errorVars()) {
                    epsContext[errVar] = 0.0;
                }

                // Set this node's error symbol to 1
                std::string errSym = "eps_" + std::to_string(node.id);
                epsContext[errSym] = 1.0;

                // Evaluate the final error expression with this configuration
                double errorContribution = std::abs(evaluateErrorExpr(ad.expr, ad.expr.root(), epsContext));

                // Apply filtering based on --show-all-instructions flag
                if (!opts.showAllInstructions && errorContribution < 1e-20) {
                    continue;  // Skip near-zero contributions
                }

                // Calculate percentage contribution
                double percentage = (totalError > 0.0) ? (errorContribution / totalError * 100.0) : 0.0;

                // Build instruction error info
                report::InstructionErrorInfo info;
                info.nodeId = node.id;
                info.instructionName = "%" + std::to_string(node.id);
                info.instructionType = nodeKindToString(node.kind);
                info.errorContribution = errorContribution;
                info.percentageContribution = percentage;
                info.irRepresentation = generateIRRepresentation(node);
                info.sourceLocation = node.loc;

                perInstructionErrors.push_back(info);
            }

            // Sort by error contribution (descending)
            std::sort(perInstructionErrors.begin(), perInstructionErrors.end(),
                [](const report::InstructionErrorInfo& a, const report::InstructionErrorInfo& b) {
                    return a.errorContribution > b.errorContribution;
                });
        }

        // 6. Output results
        if (opts.jsonToStdout) {
            // Print JSON to stdout
            report::Reporter jsonReporter(std::cout);
            report::JSONReportData jsonData {
                .filename = opts.inputFile,
                .function = g.getFunctionName(),
                .inputDomains = domain,
                .graph = g,
                .result = result,
                .perInstructionErrors = perInstructionErrors
            };
            jsonReporter.printJSON(jsonData);
        } else {
            // Write JSON to file
            std::ofstream jsonFile(opts.jsonOutputFile);
            if (!jsonFile) {
                std::cerr << "Error: Could not open output file: " << opts.jsonOutputFile << std::endl;
                return false;
            }
            report::Reporter jsonReporter(jsonFile);
            report::JSONReportData jsonData {
                .filename = opts.inputFile,
                .function = g.getFunctionName(),
                .inputDomains = domain,
                .graph = g,
                .result = result,
                .perInstructionErrors = perInstructionErrors
            };
            jsonReporter.printJSON(jsonData);
            jsonFile.close();

            // Also print text report to stdout
            report::Reporter textReporter(std::cout);
            textReporter.print(g, ad.expr, result, perInstructionErrors);
        }

        return true;
    }
}  // namespace driver
