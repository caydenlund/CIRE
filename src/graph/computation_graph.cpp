#include "computation_graph.hpp"

#include <ostream>
#include <queue>
#include <stdexcept>
#include <cmath>

namespace graph {

    std::size_t Shape::numElements() const {
        if (dims.empty()) return 1;  // scalar
        std::size_t count = 1;
        for (std::size_t d : dims) {
            count *= d;
        }
        return count;
    }

    double unitRoundoff(FloatPrec p) {
        switch (p) {
            case FloatPrec::F16:   return std::ldexp(1.0, -11);  // 2^-11 ≈ 4.88e-4
            case FloatPrec::BF16:  return std::ldexp(1.0, -8);   // 2^-8  ≈ 3.91e-3
            case FloatPrec::F32:   return std::ldexp(1.0, -24);  // 2^-24 ≈ 5.96e-8
            case FloatPrec::F64:   return std::ldexp(1.0, -53);  // 2^-53 ≈ 1.11e-16
            case FloatPrec::F128:  return std::ldexp(1.0, -113); // 2^-113 ≈ 9.63e-35
            default: throw std::runtime_error("Unknown FloatPrec");
        }
    }

    int precBits(FloatPrec p) {
        switch (p) {
            case FloatPrec::F16:   return 11;
            case FloatPrec::BF16:  return 8;
            case FloatPrec::F32:   return 24;
            case FloatPrec::F64:   return 53;
            case FloatPrec::F128:  return 113;
            default: throw std::runtime_error("Unknown FloatPrec");
        }
    }

    NodeId ComputationGraph::addNode(NodeKind kind, FloatPrec prec, Shape shape, std::optional<DebugLoc> loc) {
        NodeId id = static_cast<NodeId>(_nodes.size());
        _nodes.push_back({id, std::move(kind), prec, std::move(shape), std::move(loc)});
        return id;
    }

    NodeId ComputationGraph::addInput(std::string name, FloatPrec prec, Shape shape) {
        NodeId id = addNode(InputVarNode {name}, prec, std::move(shape));
        _input_index[name] = id;
        return id;
    }

    void ComputationGraph::markOutput(NodeId id) { _outputs.push_back(id); }

    const Node& ComputationGraph::getNode(NodeId id) const {
        if (id >= _nodes.size()) throw std::out_of_range("Invalid NodeId");
        return _nodes[id];
    }

    NodeId ComputationGraph::inputNode(const std::string& name) const {
        auto it = _input_index.find(name);
        if (it == _input_index.end()) throw std::runtime_error("Unknown input variable: " + name);
        return it->second;
    }

    std::vector<NodeId> ComputationGraph::children(NodeId id) const {
        std::vector<NodeId> result;
        std::visit(Overloaded {
                           [](const InputVarNode&) {},
                           [](const ConstantNode&) {},
                           [&](const AddNode& n) { result = {n.lhs, n.rhs}; },
                           [&](const SubNode& n) { result = {n.lhs, n.rhs}; },
                           [&](const MulNode& n) { result = {n.lhs, n.rhs}; },
                           [&](const DivNode& n) { result = {n.lhs, n.rhs}; },
                           [&](const PowNode& n) { result = {n.lhs, n.rhs}; },
                           [&](const NegNode& n) { result = {n.src}; },
                           [&](const SqrtNode& n) { result = {n.src}; },
                           [&](const AbsNode& n) { result = {n.src}; },
                           [&](const SinNode& n) { result = {n.src}; },
                           [&](const CosNode& n) { result = {n.src}; },
                           [&](const ExpNode& n) { result = {n.src}; },
                           [&](const LogNode& n) { result = {n.src}; },
                           [&](const CastNode& n) { result = {n.src}; },
                           [&](const FmaNode& n) { result = {n.a, n.b, n.c}; },
                           [&](const ReduceSumNode& n) { result = {n.src}; },
                   },
                   getNode(id).kind);
        return result;
    }

    std::vector<NodeId> ComputationGraph::topoOrder() const {
        // Kahn's algorithm for topological sort
        // Note: children() returns dependencies (inputs), so we compute in-degree based on that
        std::unordered_map<NodeId, int> in_degree;

        // Initialize in-degree for all nodes
        for (const auto& node : _nodes) {
            in_degree[node.id] = 0;
        }

        // Compute in-degree: for each node, its dependencies are its "incoming edges"
        for (const auto& node : _nodes) {
            in_degree[node.id] = children(node.id).size();
        }

        // Start with nodes that have no dependencies (in-degree 0)
        std::queue<NodeId> q;
        for (auto& [id, deg] : in_degree) {
            if (deg == 0) q.push(id);
        }

        std::vector<NodeId> order;
        while (!q.empty()) {
            NodeId cur = q.front();
            q.pop();
            order.push_back(cur);

            // Find all nodes that depend on 'cur' and decrement their in-degree
            for (const auto& node : _nodes) {
                for (NodeId dep : children(node.id)) {
                    if (dep == cur) {
                        if (--in_degree[node.id] == 0) {
                            q.push(node.id);
                        }
                    }
                }
            }
        }
        return order;
    }

    void ComputationGraph::validate() const {
        if (_outputs.empty()) throw std::runtime_error("Graph has no marked outputs");
        for (NodeId out : _outputs)
            if (out >= _nodes.size()) throw std::runtime_error("Output NodeId out of range");
        // TODO: cycle detection
    }

    void ComputationGraph::dumpDot(std::ostream& os) const {
        os << "digraph computation_graph {\n";
        for (const auto& node : _nodes) {
            os << "  " << node.id << " [label=\"" << node.id << "\"];\n";
            for (NodeId child : children(node.id)) os << "  " << node.id << " -> " << child << ";\n";
        }
        os << "}\n";
    }

}  // namespace graph
