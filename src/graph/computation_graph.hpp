#pragma once

#include "node.hpp"

#include <iosfwd>
#include <string>
#include <unordered_map>
#include <vector>

namespace graph {
    class ComputationGraph {
    public:
        // --- building ---
        [[nodiscard]] NodeId addNode(NodeKind kind, FloatPrec prec, Shape shape = Shape::scalar(),
                                     std::optional<DebugLoc> loc = std::nullopt);

        [[nodiscard]] NodeId addInput(std::string name, FloatPrec prec, Shape shape = Shape::scalar());

        void markOutput(NodeId id);

        // --- querying ---
        const Node& getNode(NodeId id) const;
        Node& getNode(NodeId id);  // Mutable accessor
        std::vector<NodeId> children(NodeId id) const;
        std::vector<NodeId> topoOrder() const;
        const std::vector<Node>& nodes() const { return _nodes; }
        const std::vector<NodeId>& outputs() const { return _outputs; }

        [[nodiscard]] NodeId inputNode(const std::string& name) const;
        const std::unordered_map<std::string, NodeId>& inputs() const { return _input_index; }

        // --- metadata ---
        void setFunctionName(std::string name) { _functionName = std::move(name); }
        const std::string& getFunctionName() const { return _functionName; }

        // --- validation ---
        void validate() const;

        // --- debug ---
        void dumpDot(std::ostream& os) const;

    private:
        std::vector<Node> _nodes;
        std::vector<NodeId> _outputs;
        std::unordered_map<std::string, NodeId> _input_index;
        std::string _functionName;
    };
}  // namespace graph
