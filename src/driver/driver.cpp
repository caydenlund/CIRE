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
#include <iomanip>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <unordered_map>
#include <utility>

namespace driver {
    // Helper: Evaluate an error expression at a given point
    static double evaluateErrorExpr(const error_expr::ErrorExpr& expr, error_expr::ExprId exprId,
                                    const std::unordered_map<std::string, double>& varValues) {
        const error_expr::ExprKind& kind = expr.get(exprId);

        return std::visit(
                [&](const auto& node) -> double {
                    using T = std::decay_t<decltype(node)>;

                    if constexpr (std::is_same_v<T, error_expr::EVarExpr>) {
                        auto it = varValues.find(node.name);
                        if (it != varValues.end()) { return it->second; }
                        return 0.0;  // Unknown variable
                    } else if constexpr (std::is_same_v<T, error_expr::EErrVar>) {
                        // Look up error variable value from context (for per-node analysis)
                        auto it = varValues.find(node.name);
                        if (it != varValues.end()) { return it->second; }
                        return 1.0;  // Default: worst case
                    } else if constexpr (std::is_same_v<T, error_expr::EConst>) {
                        return node.value;
                    } else if constexpr (std::is_same_v<T, error_expr::EEpsilon>) {
                        return graph::unitRoundoff(node.prec);
                    } else if constexpr (std::is_same_v<T, error_expr::EAdd>) {
                        return evaluateErrorExpr(expr, node.lhs, varValues)
                             + evaluateErrorExpr(expr, node.rhs, varValues);
                    } else if constexpr (std::is_same_v<T, error_expr::ESub>) {
                        return evaluateErrorExpr(expr, node.lhs, varValues)
                             - evaluateErrorExpr(expr, node.rhs, varValues);
                    } else if constexpr (std::is_same_v<T, error_expr::EMul>) {
                        return evaluateErrorExpr(expr, node.lhs, varValues)
                             * evaluateErrorExpr(expr, node.rhs, varValues);
                    } else if constexpr (std::is_same_v<T, error_expr::EDiv>) {
                        double denom = evaluateErrorExpr(expr, node.rhs, varValues);
                        if (denom == 0.0) return 0.0;
                        return evaluateErrorExpr(expr, node.lhs, varValues) / denom;
                    } else if constexpr (std::is_same_v<T, error_expr::ENeg>) {
                        return -evaluateErrorExpr(expr, node.src, varValues);
                    } else if constexpr (std::is_same_v<T, error_expr::EAbs>) {
                        return std::abs(evaluateErrorExpr(expr, node.src, varValues));
                    } else if constexpr (std::is_same_v<T, error_expr::EPow>) {
                        double base = evaluateErrorExpr(expr, node.base, varValues);
                        return std::pow(base, node.exp);
                    } else {
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
                    else if constexpr (std::is_same_v<T, graph::ConstantNode>)
                        return "const";
                    else if constexpr (std::is_same_v<T, graph::AddNode>)
                        return "fadd";
                    else if constexpr (std::is_same_v<T, graph::SubNode>)
                        return "fsub";
                    else if constexpr (std::is_same_v<T, graph::MulNode>)
                        return "fmul";
                    else if constexpr (std::is_same_v<T, graph::DivNode>)
                        return "fdiv";
                    else if constexpr (std::is_same_v<T, graph::PowNode>)
                        return "pow";
                    else if constexpr (std::is_same_v<T, graph::NegNode>)
                        return "fneg";
                    else if constexpr (std::is_same_v<T, graph::SqrtNode>)
                        return "sqrt";
                    else if constexpr (std::is_same_v<T, graph::AbsNode>)
                        return "fabs";
                    else if constexpr (std::is_same_v<T, graph::SinNode>)
                        return "sin";
                    else if constexpr (std::is_same_v<T, graph::CosNode>)
                        return "cos";
                    else if constexpr (std::is_same_v<T, graph::TanNode>)
                        return "tan";
                    else if constexpr (std::is_same_v<T, graph::AsinNode>)
                        return "asin";
                    else if constexpr (std::is_same_v<T, graph::AcosNode>)
                        return "acos";
                    else if constexpr (std::is_same_v<T, graph::AtanNode>)
                        return "atan";
                    else if constexpr (std::is_same_v<T, graph::SinhNode>)
                        return "sinh";
                    else if constexpr (std::is_same_v<T, graph::CoshNode>)
                        return "cosh";
                    else if constexpr (std::is_same_v<T, graph::TanhNode>)
                        return "tanh";
                    else if constexpr (std::is_same_v<T, graph::ExpNode>)
                        return "exp";
                    else if constexpr (std::is_same_v<T, graph::LogNode>)
                        return "log";
                    else if constexpr (std::is_same_v<T, graph::CastNode>)
                        return "fpcast";
                    else if constexpr (std::is_same_v<T, graph::FmaNode>)
                        return "fma";
                    else if constexpr (std::is_same_v<T, graph::ReduceSumNode>)
                        return "reduce_sum";
                    else
                        return "unknown";
                },
                kind);
    }

    static std::string precisionToString(graph::FloatPrec prec) {
        switch (prec) {
            case graph::FloatPrec::F16:
                return "f16";
            case graph::FloatPrec::BF16:
                return "bf16";
            case graph::FloatPrec::F32:
                return "f32";
            case graph::FloatPrec::F64:
                return "f64";
            case graph::FloatPrec::F128:
                return "f128";
            default:
                return "?";
        }
    }

    // Helper: Generate IR-like representation for a node
    static std::string generateIRRepresentation(const graph::Node& node, const graph::ComputationGraph& graph) {
        std::ostringstream oss;

        // Use LLVM instruction name if available, otherwise fall back to node ID
        std::string nodeName = node.llvmName.empty() ? ("%" + std::to_string(node.id)) : node.llvmName;
        oss << nodeName << " = " << nodeKindToString(node.kind);

        // Helper lambda to get name for a node ID
        auto getNodeName = [&](graph::NodeId id) -> std::string {
            const auto& n = graph.getNode(id);
            return n.llvmName.empty() ? ("%" + std::to_string(id)) : n.llvmName;
        };

        // Add operand information
        std::visit(
                [&](const auto& n) {
                    using T = std::decay_t<decltype(n)>;
                    if constexpr (std::is_same_v<T, graph::InputVarNode>) {
                        oss << " " << n.name;
                    } else if constexpr (std::is_same_v<T, graph::ConstantNode>) {
                        oss << " " << n.value;
                    } else if constexpr (std::is_same_v<T, graph::AddNode> || std::is_same_v<T, graph::SubNode>
                                         || std::is_same_v<T, graph::MulNode> || std::is_same_v<T, graph::DivNode>
                                         || std::is_same_v<T, graph::PowNode>) {
                        oss << " " << getNodeName(n.lhs) << ", " << getNodeName(n.rhs);
                    } else if constexpr (std::is_same_v<T, graph::NegNode> || std::is_same_v<T, graph::SqrtNode>
                                         || std::is_same_v<T, graph::AbsNode> || std::is_same_v<T, graph::SinNode>
                                         || std::is_same_v<T, graph::CosNode> || std::is_same_v<T, graph::TanNode>
                                         || std::is_same_v<T, graph::AsinNode> || std::is_same_v<T, graph::AcosNode>
                                         || std::is_same_v<T, graph::AtanNode> || std::is_same_v<T, graph::SinhNode>
                                         || std::is_same_v<T, graph::CoshNode> || std::is_same_v<T, graph::TanhNode>
                                         || std::is_same_v<T, graph::ExpNode> || std::is_same_v<T, graph::LogNode>) {
                        oss << " " << getNodeName(n.src);
                    } else if constexpr (std::is_same_v<T, graph::CastNode>) {
                        oss << " " << getNodeName(n.src) << " (" << precisionToString(n.from) << " to "
                            << precisionToString(n.to) << ")";
                    } else if constexpr (std::is_same_v<T, graph::FmaNode>) {
                        oss << " " << getNodeName(n.a) << ", " << getNodeName(n.b) << ", " << getNodeName(n.c);
                    } else if constexpr (std::is_same_v<T, graph::ReduceSumNode>) {
                        oss << " " << getNodeName(n.src);
                        if (n.axis.has_value()) { oss << ", axis=" << *n.axis; }
                    }
                },
                node.kind);

        return oss.str();
    }

    static bool isRoundingLeaf(const graph::Node& node) {
        return std::visit(
                [](const auto& n) -> bool {
                    using T = std::decay_t<decltype(n)>;
                    return std::is_same_v<T, graph::InputVarNode> || std::is_same_v<T, graph::ConstantNode>;
                },
                node.kind);
    }

    static bool isOutputNode(const graph::ComputationGraph& graph, graph::NodeId id) {
        return std::find(graph.outputs().begin(), graph.outputs().end(), id) != graph.outputs().end();
    }

    static std::string formatScientific(double value, int precision = 4) {
        std::ostringstream oss;
        oss << std::scientific << std::setprecision(precision) << value;
        return oss.str();
    }

    static std::string formatFixed(double value, int precision = 2) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(precision) << value;
        return oss.str();
    }

    static std::string trimWhitespace(const std::string& text) {
        const std::size_t first = text.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) return "";
        const std::size_t last = text.find_last_not_of(" \t\r\n");
        return text.substr(first, last - first + 1);
    }

    static std::string dotEscape(const std::string& text) {
        std::string escaped;
        escaped.reserve(text.size());
        for (char c : text) {
            switch (c) {
                case '\\':
                    escaped += "\\\\";
                    break;
                case '"':
                    escaped += "\\\"";
                    break;
                case '\n':
                    escaped += "\\l";
                    break;
                case '\r':
                    break;
                case '\t':
                    escaped += "    ";
                    break;
                default:
                    escaped += c;
                    break;
            }
        }
        return escaped;
    }

    static std::string htmlEscape(const std::string& text) {
        std::string escaped;
        escaped.reserve(text.size());
        for (char c : text) {
            switch (c) {
                case '&':
                    escaped += "&amp;";
                    break;
                case '<':
                    escaped += "&lt;";
                    break;
                case '>':
                    escaped += "&gt;";
                    break;
                case '"':
                    escaped += "&quot;";
                    break;
                default:
                    escaped += c;
                    break;
            }
        }
        return escaped;
    }

    static std::string nodeHtmlLabel(const std::vector<std::string>& lines) {
        std::ostringstream label;
        label << "<<TABLE BORDER=\"0\" CELLBORDER=\"0\" CELLSPACING=\"0\" CELLPADDING=\"2\">";

        for (std::size_t i = 0; i < lines.size(); ++i) {
            label << "<TR><TD ALIGN=\"LEFT\">";
            if (i == 0) {
                label << "<FONT FACE=\"Courier\"><B>" << htmlEscape(lines[i]) << "</B></FONT>";
            } else {
                label << "<FONT FACE=\"Helvetica\">" << htmlEscape(lines[i]) << "</FONT>";
            }
            label << "</TD></TR>";
        }

        label << "</TABLE>>";
        return label.str();
    }

    static void appendDotLabelLine(std::ostringstream& label, const std::string& line) {
        label << dotEscape(line) << "\\l";
    }

    static std::string summaryDotLabel(const optimizer::OptimizeResult& result) {
        std::ostringstream label;
        appendDotLabelLine(label, "Absolute error bound: " + formatScientific(result.upperBound, 6));
        appendDotLabelLine(label, result.provedTight ? "Status: proved tight" : "Status: sound upper bound");

        if (result.witnessInputs.empty()) {
            appendDotLabelLine(label, "Witness inputs: unavailable");
        } else {
            appendDotLabelLine(label, "Witness inputs:");

            std::vector<std::pair<std::string, double>> sortedInputs(result.witnessInputs.begin(),
                                                                     result.witnessInputs.end());
            std::sort(sortedInputs.begin(), sortedInputs.end(),
                      [](const auto& lhs, const auto& rhs) { return lhs.first < rhs.first; });

            for (const auto& [name, value] : sortedInputs) {
                appendDotLabelLine(label, "  " + name + " = " + formatScientific(value, 6));
            }
        }

        return label.str();
    }

    static std::vector<std::pair<graph::NodeId, std::string>> labelledDependencies(const graph::Node& node) {
        std::vector<std::pair<graph::NodeId, std::string>> deps;
        std::visit(graph::Overloaded {
                           [&](const graph::InputVarNode&) {},
                           [&](const graph::ConstantNode&) {},
                           [&](const graph::AddNode& n) { deps = {{n.lhs, "lhs"}, {n.rhs, "rhs"}}; },
                           [&](const graph::SubNode& n) { deps = {{n.lhs, "lhs"}, {n.rhs, "rhs"}}; },
                           [&](const graph::MulNode& n) { deps = {{n.lhs, "lhs"}, {n.rhs, "rhs"}}; },
                           [&](const graph::DivNode& n) { deps = {{n.lhs, "num"}, {n.rhs, "den"}}; },
                           [&](const graph::PowNode& n) { deps = {{n.lhs, "base"}, {n.rhs, "exp"}}; },
                           [&](const graph::NegNode& n) { deps = {{n.src, "arg"}}; },
                           [&](const graph::SqrtNode& n) { deps = {{n.src, "arg"}}; },
                           [&](const graph::AbsNode& n) { deps = {{n.src, "arg"}}; },
                           [&](const graph::SinNode& n) { deps = {{n.src, "arg"}}; },
                           [&](const graph::CosNode& n) { deps = {{n.src, "arg"}}; },
                           [&](const graph::TanNode& n) { deps = {{n.src, "arg"}}; },
                           [&](const graph::AsinNode& n) { deps = {{n.src, "arg"}}; },
                           [&](const graph::AcosNode& n) { deps = {{n.src, "arg"}}; },
                           [&](const graph::AtanNode& n) { deps = {{n.src, "arg"}}; },
                           [&](const graph::SinhNode& n) { deps = {{n.src, "arg"}}; },
                           [&](const graph::CoshNode& n) { deps = {{n.src, "arg"}}; },
                           [&](const graph::TanhNode& n) { deps = {{n.src, "arg"}}; },
                           [&](const graph::ExpNode& n) { deps = {{n.src, "arg"}}; },
                           [&](const graph::LogNode& n) { deps = {{n.src, "arg"}}; },
                           [&](const graph::CastNode& n) { deps = {{n.src, "src"}}; },
                           [&](const graph::FmaNode& n) { deps = {{n.a, "a"}, {n.b, "b"}, {n.c, "c"}}; },
                           [&](const graph::ReduceSumNode& n) { deps = {{n.src, "src"}}; },
                   },
                   node.kind);
        return deps;
    }

    static std::unordered_map<graph::NodeId, double>
    evaluateNodeValues(const graph::ComputationGraph& graph,
                       const std::unordered_map<std::string, double>& inputValues) {
        std::unordered_map<graph::NodeId, double> nodeValues;

        auto valueOf = [&](graph::NodeId id) -> double {
            auto it = nodeValues.find(id);
            return (it != nodeValues.end()) ? it->second : 0.0;
        };

        for (graph::NodeId id : graph.topoOrder()) {
            const graph::Node& node = graph.getNode(id);
            double value = std::visit(
                    graph::Overloaded {
                            [&](const graph::InputVarNode& n) -> double {
                                auto it = inputValues.find(n.name);
                                return (it != inputValues.end()) ? it->second : 0.0;
                            },
                            [&](const graph::ConstantNode& n) -> double { return n.value; },
                            [&](const graph::AddNode& n) -> double { return valueOf(n.lhs) + valueOf(n.rhs); },
                            [&](const graph::SubNode& n) -> double { return valueOf(n.lhs) - valueOf(n.rhs); },
                            [&](const graph::MulNode& n) -> double { return valueOf(n.lhs) * valueOf(n.rhs); },
                            [&](const graph::DivNode& n) -> double { return valueOf(n.lhs) / valueOf(n.rhs); },
                            [&](const graph::PowNode& n) -> double { return std::pow(valueOf(n.lhs), valueOf(n.rhs)); },
                            [&](const graph::NegNode& n) -> double { return -valueOf(n.src); },
                            [&](const graph::SqrtNode& n) -> double { return std::sqrt(valueOf(n.src)); },
                            [&](const graph::AbsNode& n) -> double { return std::abs(valueOf(n.src)); },
                            [&](const graph::SinNode& n) -> double { return std::sin(valueOf(n.src)); },
                            [&](const graph::CosNode& n) -> double { return std::cos(valueOf(n.src)); },
                            [&](const graph::TanNode& n) -> double { return std::tan(valueOf(n.src)); },
                            [&](const graph::AsinNode& n) -> double { return std::asin(valueOf(n.src)); },
                            [&](const graph::AcosNode& n) -> double { return std::acos(valueOf(n.src)); },
                            [&](const graph::AtanNode& n) -> double { return std::atan(valueOf(n.src)); },
                            [&](const graph::SinhNode& n) -> double { return std::sinh(valueOf(n.src)); },
                            [&](const graph::CoshNode& n) -> double { return std::cosh(valueOf(n.src)); },
                            [&](const graph::TanhNode& n) -> double { return std::tanh(valueOf(n.src)); },
                            [&](const graph::ExpNode& n) -> double { return std::exp(valueOf(n.src)); },
                            [&](const graph::LogNode& n) -> double { return std::log(valueOf(n.src)); },
                            [&](const graph::CastNode& n) -> double { return valueOf(n.src); },
                            [&](const graph::FmaNode& n) -> double {
                                return std::fma(valueOf(n.a), valueOf(n.b), valueOf(n.c));
                            },
                            [&](const graph::ReduceSumNode& n) -> double { return valueOf(n.src); },
                    },
                    node.kind);

            nodeValues[id] = value;
        }

        return nodeValues;
    }

    static void writeAnalysisGraphDot(std::ostream& os, const graph::ComputationGraph& graph,
                                      const optimizer::OptimizeResult& result,
                                      const std::vector<report::InstructionErrorInfo>& allInstructionErrors,
                                      const std::unordered_map<graph::NodeId, double>& witnessNodeValues) {
        std::unordered_map<graph::NodeId, const report::InstructionErrorInfo*> errorByNode;
        for (const auto& info : allInstructionErrors) { errorByNode.emplace(info.nodeId, &info); }

        os << "digraph computation_graph {\n";
        os << "  rankdir=BT;\n";
        os << "  graph [fontname=\"Helvetica\", labelloc=\"t\", label=\"";
        std::string title = graph.getFunctionName().empty()
                                  ? "CIRE annotated computation graph"
                                  : "CIRE annotated computation graph: " + graph.getFunctionName();
        os << dotEscape(title) << "\"];\n";
        os << "  newrank=true;\n";
        os << "  node [fontname=\"Helvetica\", fontsize=10, margin=\"0.08,0.06\"];\n";
        os << "  edge [fontname=\"Helvetica\", fontsize=9, color=\"#6b7280\"];\n\n";

        for (const auto& node : graph.nodes()) {
            auto errIt = errorByNode.find(node.id);
            const report::InstructionErrorInfo* errorInfo = (errIt != errorByNode.end()) ? errIt->second : nullptr;

            std::string fillcolor = "#ffffff";
            std::string color = "#374151";
            std::string shape = "box";
            std::string style = "rounded,filled";
            std::string penwidth = "1";

            if (std::holds_alternative<graph::InputVarNode>(node.kind)) {
                fillcolor = "#dbeafe";
                color = "#2563eb";
                shape = "ellipse";
            } else if (std::holds_alternative<graph::ConstantNode>(node.kind)) {
                fillcolor = "#e5e7eb";
            } else if (errorInfo != nullptr) {
                if (errorInfo->percentageContribution >= 50.0) {
                    fillcolor = "#fecaca";
                } else if (errorInfo->percentageContribution >= 10.0) {
                    fillcolor = "#fed7aa";
                } else if (errorInfo->errorContribution > 0.0) {
                    fillcolor = "#fef3c7";
                } else {
                    fillcolor = "#f3f4f6";
                }
            } else {
                fillcolor = "#f9fafb";
            }

            if (isOutputNode(graph, node.id)) {
                color = "#b91c1c";
                penwidth = "2";
            }

            std::vector<std::string> labelLines;
            const std::string irText = trimWhitespace(node.llvmIR.empty() ? generateIRRepresentation(node, graph)
                                                                          : node.llvmIR);
            labelLines.push_back("(" + precisionToString(node.prec) + ") " + irText);

            auto valueIt = witnessNodeValues.find(node.id);
            if (valueIt != witnessNodeValues.end()) {
                labelLines.push_back("Value at witness: " + formatScientific(valueIt->second, 6));
            }

            if (errorInfo != nullptr) {
                labelLines.push_back("Error contribution: " + formatScientific(errorInfo->errorContribution, 6));
                labelLines.push_back("Percentage: " + formatFixed(errorInfo->percentageContribution, 2) + "%");
            } else if (isRoundingLeaf(node)) {
                labelLines.push_back("Error contribution: none");
            } else {
                labelLines.push_back("Error contribution: unavailable");
            }

            if (node.loc.has_value()) {
                labelLines.push_back("Source: " + node.loc->file + ":" + std::to_string(node.loc->line) + ":"
                                     + std::to_string(node.loc->col));
            }

            os << "  n" << node.id << " [shape=" << shape << ", style=\"" << style << "\""
               << ", fillcolor=\"" << fillcolor << "\""
               << ", color=\"" << color << "\""
               << ", penwidth=" << penwidth << ", label=" << nodeHtmlLabel(labelLines) << "];\n";
        }

        os << "\n";
        os << "  subgraph cluster_inputs {\n";
        os << "    label=\"Inputs\";\n";
        os << "    fontname=\"Helvetica\";\n";
        os << "    color=\"#94a3b8\";\n";
        os << "    style=\"rounded\";\n";
        os << "    margin=12;\n";
        std::vector<graph::NodeId> inputNodeIds;
        for (const auto& [_, nodeId] : graph.inputs()) inputNodeIds.push_back(nodeId);
        std::sort(inputNodeIds.begin(), inputNodeIds.end());
        for (graph::NodeId nodeId : inputNodeIds) { os << "    n" << nodeId << ";\n"; }
        os << "  }\n\n";

        os << "  subgraph cluster_outputs {\n";
        os << "    label=\"" << (graph.outputs().size() == 1 ? "Output" : "Outputs") << "\";\n";
        os << "    fontname=\"Helvetica\";\n";
        os << "    color=\"#94a3b8\";\n";
        os << "    style=\"rounded\";\n";
        os << "    margin=12;\n";
        for (graph::NodeId nodeId : graph.outputs()) { os << "    n" << nodeId << ";\n"; }
        os << "  }\n";

        os << "\n";
        os << "  summary [shape=note, style=\"filled\", fillcolor=\"#f8fafc\", color=\"#64748b\", label=\""
           << summaryDotLabel(result) << "\"];\n";

        if (!graph.outputs().empty()) {
            os << "  { rank=same; ";
            for (graph::NodeId nodeId : graph.outputs()) { os << "n" << nodeId << "; "; }
            os << "summary; }\n";

            graph::NodeId lastOutput = graph.outputs().back();
            os << "  n" << lastOutput << " -> summary [style=invis, weight=100];\n";
        }

        os << "\n";
        for (const auto& node : graph.nodes()) {
            for (const auto& [dep, label] : labelledDependencies(node)) {
                os << "  n" << dep << " -> n" << node.id << " [label=\"" << dotEscape(label) << "\"];\n";
            }
        }

        os << "}\n";
    }

    bool run(const DriverOpts& opts, const frontend::Frontend& fe) {
        // 1. Parse → ComputationGraph
        frontend::FrontendOpts fopts {
                .verbose = opts.verbose,
                .targetFunction = opts.targetFunction,
        };
        graph::ComputationGraph g = fe.parse(opts.inputFile, fopts);
        g.validate();

        // 2. Load input domain (json file or var=[l,u])
        std::vector<std::string> inputVarNames;
        for (const auto& [name, _] : g.inputs()) inputVarNames.push_back(name);
        interval::InputDomain domain = interval::parseDomainArgs(opts.domainArgs, opts.defaultDomain, inputVarNames);

        // 3. Symbolic autodiff → error expression
        autodiff::AutodiffResult ad = autodiff::analyze(g);

        // 4. Select optimizer
        std::unique_ptr<optimizer::Optimizer> opt = std::make_unique<optimizer::IbexOptimizer>();

        optimizer::OptimizerOpts oopts {.timeoutSeconds = opts.timeoutSeconds, .verbose = opts.verbose};
        optimizer::OptimizeResult result = opt->maximize(ad.expr, domain, g, ad.symbolicVal, oopts);

        // 5. Compute per-node error contributions
        std::vector<report::InstructionErrorInfo> allInstructionErrors;
        std::vector<report::InstructionErrorInfo> perInstructionErrors;
        std::unordered_map<graph::NodeId, double> witnessNodeValues;

        if (!result.witnessInputs.empty()) {
            // Build evaluation context with witness inputs
            std::unordered_map<std::string, double> evalContext = result.witnessInputs;

            // Evaluate node values at the witness point, including v_<node-id>
            // placeholders used by transcendental error rules.
            witnessNodeValues = evaluateNodeValues(g, result.witnessInputs);
            for (const auto& [nodeId, value] : witnessNodeValues) {
                evalContext["v_" + std::to_string(nodeId)] = value;
            }

            // Compute total error for percentage calculation
            double totalError = result.upperBound;

            // Compute error contribution for each node by evaluating its error symbol's coefficient
            for (const graph::Node& node : g.nodes()) {
                if (isRoundingLeaf(node)) {
                    continue;  // Inputs and constants don't introduce rounding error
                }

                // To find this node's contribution to the final error:
                // Set eps_N = 1 and all other eps = 0, then evaluate the error expression
                std::unordered_map<std::string, double> epsContext = evalContext;

                // Set all error symbols to 0
                for (const auto& errVar : ad.expr.errorVars()) { epsContext[errVar] = 0.0; }

                // Set this node's error symbol to 1
                std::string errSym = "eps_" + std::to_string(node.id);
                epsContext[errSym] = 1.0;

                // Evaluate the final error expression with this configuration
                double errorContribution = std::abs(evaluateErrorExpr(ad.expr, ad.expr.root(), epsContext));

                // Calculate percentage contribution
                double percentage = (totalError > 0.0) ? (errorContribution / totalError * 100.0) : 0.0;

                // Build instruction error info
                report::InstructionErrorInfo info;
                info.nodeId = node.id;
                // Use LLVM name if available, otherwise use node ID
                info.instructionName = node.llvmName.empty() ? ("%" + std::to_string(node.id)) : node.llvmName;
                info.instructionType = nodeKindToString(node.kind);
                info.errorContribution = errorContribution;
                info.percentageContribution = percentage;
                // Use actual LLVM IR if available, otherwise generate synthetic representation
                info.irRepresentation = node.llvmIR.empty() ? generateIRRepresentation(node, g) : node.llvmIR;
                info.sourceLocation = node.loc;

                allInstructionErrors.push_back(info);
                if (opts.showAllInstructions || errorContribution >= 1e-20) { perInstructionErrors.push_back(info); }
            }

            // Sort by error contribution (descending)
            std::sort(allInstructionErrors.begin(), allInstructionErrors.end(),
                      [](const report::InstructionErrorInfo& a, const report::InstructionErrorInfo& b) {
                          return a.errorContribution > b.errorContribution;
                      });
            std::sort(perInstructionErrors.begin(), perInstructionErrors.end(),
                      [](const report::InstructionErrorInfo& a, const report::InstructionErrorInfo& b) {
                          return a.errorContribution > b.errorContribution;
                      });
        }

        // 6. Write annotated graph, if requested
        if (!opts.outputGraphFile.empty()) {
            std::ofstream graphFile(opts.outputGraphFile);
            if (!graphFile) {
                std::cerr << "Error: Could not open graph output file: " << opts.outputGraphFile << std::endl;
                return false;
            }
            writeAnalysisGraphDot(graphFile, g, result, allInstructionErrors, witnessNodeValues);
        }

        // 7. Output results
        if (opts.jsonToStdout) {
            // Print JSON to stdout
            report::Reporter jsonReporter(std::cout);
            report::JSONReportData jsonData {.filename = opts.inputFile,
                                             .function = g.getFunctionName(),
                                             .inputDomains = domain,
                                             .graph = g,
                                             .result = result,
                                             .perInstructionErrors = perInstructionErrors};
            jsonReporter.printJSON(jsonData);
        } else {
            // Write JSON to file
            std::ofstream jsonFile(opts.jsonOutputFile);
            if (!jsonFile) {
                std::cerr << "Error: Could not open output file: " << opts.jsonOutputFile << std::endl;
                return false;
            }
            report::Reporter jsonReporter(jsonFile);
            report::JSONReportData jsonData {.filename = opts.inputFile,
                                             .function = g.getFunctionName(),
                                             .inputDomains = domain,
                                             .graph = g,
                                             .result = result,
                                             .perInstructionErrors = perInstructionErrors};
            jsonReporter.printJSON(jsonData);
            jsonFile.close();

            // Also print text report to stdout
            report::Reporter textReporter(std::cout);
            textReporter.print(g, ad.expr, result, perInstructionErrors, opts.detailed, domain);
        }

        return true;
    }
}  // namespace driver
