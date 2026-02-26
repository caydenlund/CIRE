#include "error_expr/error_expr.hpp"
#include "graph/node.hpp"

#include <cmath>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace autodiff::detail {
    using graph::Overloaded;
    using namespace error_expr;  // TODO: Remove
    using namespace graph;       // TODO: Remove

    static std::string errSym(NodeId id) { return "eps_" + std::to_string(id); }

    std::pair<ExprId, ExprId> applyRule(const Node& node, ErrorExpr& arena,
                                        const std::unordered_map<NodeId, ExprId>& val,
                                        const std::unordered_map<NodeId, ExprId>& err) {
        return std::visit(
                Overloaded {

                        [&](const InputVarNode& n) -> std::pair<ExprId, ExprId> {
                            return {arena.makeVar(n.name), arena.makeConst(0.0)};
                        },

                        [&](const ConstantNode&) -> std::pair<ExprId, ExprId> {
                            return {arena.makeConst(0.0), arena.makeConst(0.0)};
                        },

                        [&](const AddNode& n) -> std::pair<ExprId, ExprId> {
                            ExprId vz = arena.makeAdd(val.at(n.lhs), val.at(n.rhs));
                            ExprId ez = arena.makeAdd(arena.makeAdd(err.at(n.lhs), err.at(n.rhs)),
                                                      arena.makeMul(vz, arena.makeErrVar(errSym(node.id))));
                            return {vz, ez};
                        },

                        [&](const SubNode& n) -> std::pair<ExprId, ExprId> {
                            ExprId vz = arena.makeSub(val.at(n.lhs), val.at(n.rhs));
                            ExprId ez = arena.makeAdd(arena.makeSub(err.at(n.lhs), err.at(n.rhs)),
                                                      arena.makeMul(vz, arena.makeErrVar(errSym(node.id))));
                            return {vz, ez};
                        },

                        [&](const MulNode& n) -> std::pair<ExprId, ExprId> {
                            ExprId vz = arena.makeMul(val.at(n.lhs), val.at(n.rhs));
                            ExprId propagated = arena.makeAdd(arena.makeMul(val.at(n.rhs), err.at(n.lhs)),
                                                              arena.makeMul(val.at(n.lhs), err.at(n.rhs)));
                            ExprId ez = arena.makeAdd(propagated, arena.makeMul(vz, arena.makeErrVar(errSym(node.id))));
                            return {vz, ez};
                        },

                        [&](const DivNode& n) -> std::pair<ExprId, ExprId> {
                            // err(x/y) ≈ err(x)/val(y) - val(x)*err(y)/val(y)^2 + val(z)*ε
                            ExprId vz = arena.makeMul(val.at(n.lhs), arena.add(EPow {val.at(n.rhs), -1}));
                            // simplified: propagated ≈ (err(x) - vz*err(y)) / val(y)
                            ExprId propagated = arena.makeMul(
                                    arena.makeSub(err.at(n.lhs), arena.makeMul(vz, err.at(n.rhs))),
                                    arena.add(EPow {val.at(n.rhs), -1}));
                            ExprId ez = arena.makeAdd(propagated, arena.makeMul(vz, arena.makeErrVar(errSym(node.id))));
                            return {vz, ez};
                        },

                        [&](const CastNode& n) -> std::pair<ExprId, ExprId> {
                            if (precBits(n.to) >= precBits(n.from)) {
                                // Widening: exact
                                return {val.at(n.src), err.at(n.src)};
                            }
                            // Narrowing: new rounding error
                            ExprId ez = arena.makeAdd(err.at(n.src),
                                                      arena.makeMul(val.at(n.src), arena.makeErrVar(errSym(node.id))));
                            return {val.at(n.src), ez};
                        },

                        [&](const FmaNode& n) -> std::pair<ExprId, ExprId> {
                            // Single rounding: fl(a*b + c)
                            ExprId vz = arena.makeAdd(arena.makeMul(val.at(n.a), val.at(n.b)), val.at(n.c));
                            ExprId propagated = arena.makeAdd(arena.makeAdd(arena.makeMul(val.at(n.b), err.at(n.a)),
                                                                            arena.makeMul(val.at(n.a), err.at(n.b))),
                                                              err.at(n.c));
                            ExprId ez = arena.makeAdd(propagated, arena.makeMul(vz, arena.makeErrVar(errSym(node.id))));
                            return {vz, ez};
                        },

                        [&](const ReduceSumNode& n) -> std::pair<ExprId, ExprId> {
                            std::size_t k = node.shape.numElements();
                            double log2k = (k > 0) ? std::log2(static_cast<double>(k)) : 1.0;
                            ExprId vz = val.at(n.src);
                            ExprId ez = arena.makeAdd(err.at(n.src),
                                                      arena.makeMul(arena.makeMul(vz, arena.makeConst(log2k)),
                                                                    arena.makeEpsilon(node.prec)));
                            return {vz, ez};
                        },

                        // Transcendentals: err(f(x)) ≈ f'(x)*err(x) + f(x)*ε
                        [&](const SinNode& n) -> std::pair<ExprId, ExprId> {
                            ExprId vz = arena.add(
                                    EVarExpr {"sin(" + std::get<EVarExpr>(arena.get(val.at(n.src))).name + ")"});
                            ExprId df = arena.add(
                                    EVarExpr {"cos(" + std::get<EVarExpr>(arena.get(val.at(n.src))).name + ")"});
                            ExprId ez = arena.makeAdd(arena.makeMul(df, err.at(n.src)),
                                                      arena.makeMul(vz, arena.makeErrVar(errSym(node.id))));
                            return {vz, ez};
                        },

                        // TODO: NegNode, SqrtNode, AbsNode, CosNode, ExpNode, LogNode
                        // follow the same pattern; omitted for brevity

                        [&](const auto&) -> std::pair<ExprId, ExprId> {
                            throw std::runtime_error("applyRule: unhandled node kind");
                        },

                },
                node.kind);
    }
}  // namespace autodiff::detail
