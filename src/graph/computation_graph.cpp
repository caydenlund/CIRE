#include "computation_graph.hpp"

#include <cmath>
#include <iomanip>
#include <ostream>
#include <queue>
#include <sstream>
#include <stdexcept>

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

    Node& ComputationGraph::getNode(NodeId id) {
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
                           [&](const TanNode& n) { result = {n.src}; },
                           [&](const AsinNode& n) { result = {n.src}; },
                           [&](const AcosNode& n) { result = {n.src}; },
                           [&](const AtanNode& n) { result = {n.src}; },
                           [&](const SinhNode& n) { result = {n.src}; },
                           [&](const CoshNode& n) { result = {n.src}; },
                           [&](const TanhNode& n) { result = {n.src}; },
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

        // Cycle detection: verify topological sort includes all nodes
        std::vector<NodeId> order = topoOrder();
        if (order.size() != _nodes.size()) {
            throw std::runtime_error("Graph contains a cycle (topological sort failed)");
        }
    }

    void ComputationGraph::dumpDot(std::ostream& os) const {
        os << "digraph computation_graph {\n";
        os << "  rankdir=BT;\n";  // Bottom to top (inputs at bottom, outputs at top)
        os << "  node [fontname=\"Helvetica\"];\n";
        os << "  edge [fontname=\"Helvetica\"];\n\n";

        // Helper to get precision string
        auto precStr = [](FloatPrec p) -> std::string {
            switch (p) {
                case FloatPrec::F16: return "f16";
                case FloatPrec::BF16: return "bf16";
                case FloatPrec::F32: return "f32";
                case FloatPrec::F64: return "f64";
                case FloatPrec::F128: return "f128";
                default: return "?";
            }
        };

        // Check if a node is an output
        auto isOutput = [&](NodeId id) {
            for (NodeId out : _outputs) {
                if (out == id) return true;
            }
            return false;
        };

        // Draw nodes
        for (const auto& node : _nodes) {
            os << "  n" << node.id << " [";

            // Create label based on node type
            std::string label;
            std::string shape = "box";
            std::string color = "black";
            std::string fillcolor = "white";
            std::string style = "filled";

            std::visit(Overloaded{
                [&](const InputVarNode& n) {
                    label = n.name + "\\n(" + precStr(node.prec) + ")";
                    shape = "ellipse";
                    fillcolor = "lightblue";
                    color = "blue";
                },
                [&](const ConstantNode& n) {
                    std::ostringstream oss;
                    oss << std::fixed << std::setprecision(4) << n.value;
                    label = oss.str() + "\\n(" + precStr(node.prec) + ")";
                    shape = "box";
                    fillcolor = "lightgray";
                },
                [&](const AddNode&) {
                    label = "+\\n(" + precStr(node.prec) + ")";
                    fillcolor = "lightyellow";
                },
                [&](const SubNode&) {
                    label = "-\\n(" + precStr(node.prec) + ")";
                    fillcolor = "lightyellow";
                },
                [&](const MulNode&) {
                    label = "*\\n(" + precStr(node.prec) + ")";
                    fillcolor = "lightgreen";
                },
                [&](const DivNode&) {
                    label = "/\\n(" + precStr(node.prec) + ")";
                    fillcolor = "lightgreen";
                },
                [&](const PowNode&) {
                    label = "pow\\n(" + precStr(node.prec) + ")";
                    fillcolor = "lightgreen";
                },
                [&](const NegNode&) {
                    label = "neg\\n(" + precStr(node.prec) + ")";
                    fillcolor = "lightyellow";
                },
                [&](const SqrtNode&) {
                    label = "sqrt\\n(" + precStr(node.prec) + ")";
                    fillcolor = "lightpink";
                },
                [&](const AbsNode&) {
                    label = "abs\\n(" + precStr(node.prec) + ")";
                    fillcolor = "lightpink";
                },
                [&](const SinNode&) {
                    label = "sin\\n(" + precStr(node.prec) + ")";
                    fillcolor = "lightpink";
                },
                [&](const CosNode&) {
                    label = "cos\\n(" + precStr(node.prec) + ")";
                    fillcolor = "lightpink";
                },
                [&](const ExpNode&) {
                    label = "exp\\n(" + precStr(node.prec) + ")";
                    fillcolor = "lightpink";
                },
                [&](const LogNode&) {
                    label = "log\\n(" + precStr(node.prec) + ")";
                    fillcolor = "lightpink";
                },
                [&](const TanNode&) {
                    label = "tan\\n(" + precStr(node.prec) + ")";
                    fillcolor = "lightpink";
                },
                [&](const AsinNode&) {
                    label = "asin\\n(" + precStr(node.prec) + ")";
                    fillcolor = "lightpink";
                },
                [&](const AcosNode&) {
                    label = "acos\\n(" + precStr(node.prec) + ")";
                    fillcolor = "lightpink";
                },
                [&](const AtanNode&) {
                    label = "atan\\n(" + precStr(node.prec) + ")";
                    fillcolor = "lightpink";
                },
                [&](const SinhNode&) {
                    label = "sinh\\n(" + precStr(node.prec) + ")";
                    fillcolor = "lightpink";
                },
                [&](const CoshNode&) {
                    label = "cosh\\n(" + precStr(node.prec) + ")";
                    fillcolor = "lightpink";
                },
                [&](const TanhNode&) {
                    label = "tanh\\n(" + precStr(node.prec) + ")";
                    fillcolor = "lightpink";
                },
                [&](const CastNode& n) {
                    label = "cast\\n" + precStr(n.from) + "→" + precStr(n.to);
                    fillcolor = "orange";
                },
                [&](const FmaNode&) {
                    label = "fma\\n(" + precStr(node.prec) + ")";
                    fillcolor = "lightcyan";
                },
                [&](const ReduceSumNode&) {
                    label = "reduce_sum\\n(" + precStr(node.prec) + ")";
                    fillcolor = "lightcyan";
                },
            }, node.kind);

            // Mark outputs with double border
            if (isOutput(node.id)) {
                style = "filled,bold";
                color = "red";
                label += "\\n[OUTPUT]";
            }

            os << "label=\"" << label << "\", ";
            os << "shape=" << shape << ", ";
            os << "color=" << color << ", ";
            os << "fillcolor=" << fillcolor << ", ";
            os << "style=\"" << style << "\"";
            os << "];\n";
        }

        os << "\n";

        // Draw edges
        for (const auto& node : _nodes) {
            std::vector<NodeId> deps = children(node.id);

            // For binary operations, label edges with position
            if (deps.size() == 2) {
                os << "  n" << deps[0] << " -> n" << node.id << " [label=\"L\"];\n";
                os << "  n" << deps[1] << " -> n" << node.id << " [label=\"R\"];\n";
            } else if (deps.size() == 3) {
                // For ternary operations like FMA
                os << "  n" << deps[0] << " -> n" << node.id << " [label=\"A\"];\n";
                os << "  n" << deps[1] << " -> n" << node.id << " [label=\"B\"];\n";
                os << "  n" << deps[2] << " -> n" << node.id << " [label=\"C\"];\n";
            } else {
                // For unary operations or single dependencies
                for (NodeId child : deps) {
                    os << "  n" << child << " -> n" << node.id << ";\n";
                }
            }
        }

        os << "}\n";
    }

}  // namespace graph
