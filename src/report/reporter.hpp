#pragma once
#include "error_expr/error_expr.hpp"
#include "graph/computation_graph.hpp"
#include "interval/interval.hpp"
#include "optimizer/optimizer.hpp"

#include <ostream>
#include <string>
#include <vector>

namespace report {
    struct InstructionErrorInfo {
        graph::NodeId nodeId;
        std::string instructionName;
        std::string instructionType;
        double errorContribution;
        double percentageContribution;
        std::string irRepresentation;
        std::optional<graph::DebugLoc> sourceLocation;
    };

    struct JSONReportData {
        std::string filename;
        std::string function;
        interval::InputDomain inputDomains;
        const graph::ComputationGraph& graph;
        const optimizer::OptimizeResult& result;
        std::vector<InstructionErrorInfo> perInstructionErrors;
    };

    class Reporter {
    public:
        explicit Reporter(std::ostream& out) : _out(out) {}

        void print(const graph::ComputationGraph& graph, const error_expr::ErrorExpr& expr,
                   const optimizer::OptimizeResult& result,
                   const std::vector<InstructionErrorInfo>& perInstructionErrors = {},
                   bool detailed = false,
                   const interval::InputDomain& inputDomains = interval::InputDomain {}) const;

        void printOptimizerDetails(const optimizer::OptimizeResult& result) const;
        void printComputationExpression(const graph::ComputationGraph& graph) const;

        void printJSON(const JSONReportData& data) const;

    private:
        std::ostream& _out;
    };
}  // namespace report
