#include "error_expr/error_expr.hpp"
#include "graph/node.hpp"

#include <cmath>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace autodiff::detail {
    using graph::Overloaded;

    static std::string errSym(graph::NodeId id) { return "eps_" + std::to_string(id); }

    std::pair<error_expr::ExprId, error_expr::ExprId> applyRule(
            const graph::Node& node, error_expr::ErrorExpr& arena,
            const std::unordered_map<graph::NodeId, error_expr::ExprId>& val,
            const std::unordered_map<graph::NodeId, error_expr::ExprId>& err) {
        return std::visit(
                Overloaded {

                        [&](const graph::InputVarNode& n) -> std::pair<error_expr::ExprId, error_expr::ExprId> {
                            return {arena.makeVar(n.name), arena.makeConst(0.0)};
                        },

                        [&](const graph::ConstantNode& n) -> std::pair<error_expr::ExprId, error_expr::ExprId> {
                            return {arena.makeConst(n.value), arena.makeConst(0.0)};
                        },

                        [&](const graph::AddNode& n) -> std::pair<error_expr::ExprId, error_expr::ExprId> {
                            error_expr::ExprId vz = arena.makeAdd(val.at(n.lhs), val.at(n.rhs));
                            error_expr::ExprId u = arena.makeEpsilon(node.prec);
                            error_expr::ExprId ez = arena.makeAdd(arena.makeAdd(err.at(n.lhs), err.at(n.rhs)),
                                                      arena.makeMul(arena.makeMul(vz, arena.makeErrVar(errSym(node.id))), u));
                            return {vz, ez};
                        },

                        [&](const graph::SubNode& n) -> std::pair<error_expr::ExprId, error_expr::ExprId> {
                            error_expr::ExprId vz = arena.makeSub(val.at(n.lhs), val.at(n.rhs));
                            error_expr::ExprId u = arena.makeEpsilon(node.prec);
                            error_expr::ExprId ez = arena.makeAdd(arena.makeSub(err.at(n.lhs), err.at(n.rhs)),
                                                      arena.makeMul(arena.makeMul(vz, arena.makeErrVar(errSym(node.id))), u));
                            return {vz, ez};
                        },

                        [&](const graph::MulNode& n) -> std::pair<error_expr::ExprId, error_expr::ExprId> {
                            error_expr::ExprId vz = arena.makeMul(val.at(n.lhs), val.at(n.rhs));
                            error_expr::ExprId propagated = arena.makeAdd(arena.makeMul(val.at(n.rhs), err.at(n.lhs)),
                                                              arena.makeMul(val.at(n.lhs), err.at(n.rhs)));
                            error_expr::ExprId u = arena.makeEpsilon(node.prec);
                            error_expr::ExprId ez = arena.makeAdd(propagated, arena.makeMul(arena.makeMul(vz, arena.makeErrVar(errSym(node.id))), u));
                            return {vz, ez};
                        },

                        [&](const graph::DivNode& n) -> std::pair<error_expr::ExprId, error_expr::ExprId> {
                            // err(x/y) ≈ err(x)/val(y) - val(x)*err(y)/val(y)^2 + val(z)*ε*u
                            error_expr::ExprId vz = arena.makeMul(val.at(n.lhs), arena.add(error_expr::EPow {val.at(n.rhs), -1}));
                            // simplified: propagated ≈ (err(x) - vz*err(y)) / val(y)
                            error_expr::ExprId propagated = arena.makeMul(
                                    arena.makeSub(err.at(n.lhs), arena.makeMul(vz, err.at(n.rhs))),
                                    arena.add(error_expr::EPow {val.at(n.rhs), -1}));
                            error_expr::ExprId u = arena.makeEpsilon(node.prec);
                            error_expr::ExprId ez = arena.makeAdd(propagated, arena.makeMul(arena.makeMul(vz, arena.makeErrVar(errSym(node.id))), u));
                            return {vz, ez};
                        },

                        [&](const graph::CastNode& n) -> std::pair<error_expr::ExprId, error_expr::ExprId> {
                            if (graph::precBits(n.to) >= graph::precBits(n.from)) {
                                // Widening: exact
                                return {val.at(n.src), err.at(n.src)};
                            }
                            // Narrowing: new rounding error
                            error_expr::ExprId u = arena.makeEpsilon(node.prec);
                            error_expr::ExprId ez = arena.makeAdd(err.at(n.src),
                                                      arena.makeMul(arena.makeMul(val.at(n.src), arena.makeErrVar(errSym(node.id))), u));
                            return {val.at(n.src), ez};
                        },

                        [&](const graph::FmaNode& n) -> std::pair<error_expr::ExprId, error_expr::ExprId> {
                            // Single rounding: fl(a*b + c)
                            error_expr::ExprId vz = arena.makeAdd(arena.makeMul(val.at(n.a), val.at(n.b)), val.at(n.c));
                            error_expr::ExprId propagated = arena.makeAdd(arena.makeAdd(arena.makeMul(val.at(n.b), err.at(n.a)),
                                                                            arena.makeMul(val.at(n.a), err.at(n.b))),
                                                              err.at(n.c));
                            error_expr::ExprId u = arena.makeEpsilon(node.prec);
                            error_expr::ExprId ez = arena.makeAdd(propagated, arena.makeMul(arena.makeMul(vz, arena.makeErrVar(errSym(node.id))), u));
                            return {vz, ez};
                        },

                        [&](const graph::ReduceSumNode& n) -> std::pair<error_expr::ExprId, error_expr::ExprId> {
                            std::size_t k = node.shape.numElements();
                            double log2k = (k > 0) ? std::log2(static_cast<double>(k)) : 1.0;
                            error_expr::ExprId vz = val.at(n.src);
                            error_expr::ExprId ez = arena.makeAdd(err.at(n.src),
                                                      arena.makeMul(arena.makeMul(vz, arena.makeConst(log2k)),
                                                                    arena.makeEpsilon(node.prec)));
                            return {vz, ez};
                        },

                        // Simple operations
                        [&](const graph::NegNode& n) -> std::pair<error_expr::ExprId, error_expr::ExprId> {
                            error_expr::ExprId vz = arena.makeSub(arena.makeConst(0.0), val.at(n.src));
                            error_expr::ExprId ez = arena.makeSub(arena.makeConst(0.0), err.at(n.src));
                            return {vz, ez};
                        },

                        // Transcendentals: err(f(x)) ≈ f'(x)*err(x) + f(x)*ε
                        // Create a placeholder variable for this node's value
                        [&](const graph::SinNode& n) -> std::pair<error_expr::ExprId, error_expr::ExprId> {
                            std::string nodeName = "v_" + std::to_string(node.id);
                            error_expr::ExprId vz = arena.add(error_expr::EVarExpr {nodeName});
                            // d/dx[sin(x)] = cos(x), bounded by [-1, 1]
                            error_expr::ExprId df = arena.makeConst(1.0);  // Worst-case derivative magnitude
                            error_expr::ExprId u = arena.makeEpsilon(node.prec);
                            error_expr::ExprId ez = arena.makeAdd(arena.makeMul(df, err.at(n.src)),
                                                      arena.makeMul(arena.makeMul(vz, arena.makeErrVar(errSym(node.id))), u));
                            return {vz, ez};
                        },

                        [&](const graph::CosNode& n) -> std::pair<error_expr::ExprId, error_expr::ExprId> {
                            std::string nodeName = "v_" + std::to_string(node.id);
                            error_expr::ExprId vz = arena.add(error_expr::EVarExpr {nodeName});
                            // d/dx[cos(x)] = -sin(x), magnitude bounded by [-1, 1]
                            error_expr::ExprId df = arena.makeConst(1.0);  // Worst-case derivative magnitude
                            error_expr::ExprId u = arena.makeEpsilon(node.prec);
                            error_expr::ExprId ez = arena.makeAdd(arena.makeMul(df, err.at(n.src)),
                                                      arena.makeMul(arena.makeMul(vz, arena.makeErrVar(errSym(node.id))), u));
                            return {vz, ez};
                        },

                        [&](const graph::ExpNode& n) -> std::pair<error_expr::ExprId, error_expr::ExprId> {
                            std::string nodeName = "v_" + std::to_string(node.id);
                            error_expr::ExprId vz = arena.add(error_expr::EVarExpr {nodeName});
                            // d/dx[exp(x)] = exp(x)
                            error_expr::ExprId df = vz;
                            error_expr::ExprId u = arena.makeEpsilon(node.prec);
                            error_expr::ExprId ez = arena.makeAdd(arena.makeMul(df, err.at(n.src)),
                                                      arena.makeMul(arena.makeMul(vz, arena.makeErrVar(errSym(node.id))), u));
                            return {vz, ez};
                        },

                        [&](const graph::LogNode& n) -> std::pair<error_expr::ExprId, error_expr::ExprId> {
                            std::string nodeName = "v_" + std::to_string(node.id);
                            error_expr::ExprId vz = arena.add(error_expr::EVarExpr {nodeName});
                            // d/dx[ln(x)] = 1/x
                            error_expr::ExprId df = arena.add(error_expr::EPow {val.at(n.src), -1});
                            error_expr::ExprId u = arena.makeEpsilon(node.prec);
                            error_expr::ExprId ez = arena.makeAdd(arena.makeMul(df, err.at(n.src)),
                                                      arena.makeMul(arena.makeMul(vz, arena.makeErrVar(errSym(node.id))), u));
                            return {vz, ez};
                        },

                        [&](const graph::SqrtNode& n) -> std::pair<error_expr::ExprId, error_expr::ExprId> {
                            std::string nodeName = "v_" + std::to_string(node.id);
                            error_expr::ExprId vz = arena.add(error_expr::EVarExpr {nodeName});
                            // d/dx[sqrt(x)] = 1/(2*sqrt(x))
                            error_expr::ExprId df = arena.makeMul(
                                    arena.makeConst(0.5),
                                    arena.add(error_expr::EPow {vz, -1}));
                            error_expr::ExprId u = arena.makeEpsilon(node.prec);
                            error_expr::ExprId ez = arena.makeAdd(arena.makeMul(df, err.at(n.src)),
                                                      arena.makeMul(arena.makeMul(vz, arena.makeErrVar(errSym(node.id))), u));
                            return {vz, ez};
                        },

                        [&](const graph::AbsNode& n) -> std::pair<error_expr::ExprId, error_expr::ExprId> {
                            std::string nodeName = "v_" + std::to_string(node.id);
                            error_expr::ExprId vz = arena.add(error_expr::EVarExpr {nodeName});
                            // d/dx[|x|] = sign(x), but for worst-case we use 1
                            error_expr::ExprId df = arena.makeConst(1.0);
                            error_expr::ExprId u = arena.makeEpsilon(node.prec);
                            error_expr::ExprId ez = arena.makeAdd(arena.makeMul(df, err.at(n.src)),
                                                      arena.makeMul(arena.makeMul(vz, arena.makeErrVar(errSym(node.id))), u));
                            return {vz, ez};
                        },

                        [&](const graph::TanNode& n) -> std::pair<error_expr::ExprId, error_expr::ExprId> {
                            std::string nodeName = "v_" + std::to_string(node.id);
                            error_expr::ExprId vz = arena.add(error_expr::EVarExpr {nodeName});
                            // d/dx[tan(x)] = 1/cos^2(x), bounded by >= 1
                            error_expr::ExprId df = arena.makeConst(1.0);  // Worst-case derivative magnitude
                            error_expr::ExprId u = arena.makeEpsilon(node.prec);
                            error_expr::ExprId ez = arena.makeAdd(arena.makeMul(df, err.at(n.src)),
                                                      arena.makeMul(arena.makeMul(vz, arena.makeErrVar(errSym(node.id))), u));
                            return {vz, ez};
                        },

                        [&](const graph::AsinNode& n) -> std::pair<error_expr::ExprId, error_expr::ExprId> {
                            std::string nodeName = "v_" + std::to_string(node.id);
                            error_expr::ExprId vz = arena.add(error_expr::EVarExpr {nodeName});
                            // d/dx[asin(x)] = 1/sqrt(1-x^2), use conservative bound
                            error_expr::ExprId df = arena.makeConst(2.0);  // Conservative bound
                            error_expr::ExprId u = arena.makeEpsilon(node.prec);
                            error_expr::ExprId ez = arena.makeAdd(arena.makeMul(df, err.at(n.src)),
                                                      arena.makeMul(arena.makeMul(vz, arena.makeErrVar(errSym(node.id))), u));
                            return {vz, ez};
                        },

                        [&](const graph::AcosNode& n) -> std::pair<error_expr::ExprId, error_expr::ExprId> {
                            std::string nodeName = "v_" + std::to_string(node.id);
                            error_expr::ExprId vz = arena.add(error_expr::EVarExpr {nodeName});
                            // d/dx[acos(x)] = -1/sqrt(1-x^2), use conservative bound
                            error_expr::ExprId df = arena.makeConst(2.0);  // Conservative bound
                            error_expr::ExprId u = arena.makeEpsilon(node.prec);
                            error_expr::ExprId ez = arena.makeAdd(arena.makeMul(df, err.at(n.src)),
                                                      arena.makeMul(arena.makeMul(vz, arena.makeErrVar(errSym(node.id))), u));
                            return {vz, ez};
                        },

                        [&](const graph::AtanNode& n) -> std::pair<error_expr::ExprId, error_expr::ExprId> {
                            std::string nodeName = "v_" + std::to_string(node.id);
                            error_expr::ExprId vz = arena.add(error_expr::EVarExpr {nodeName});
                            // d/dx[atan(x)] = 1/(1+x^2), bounded by [0, 1]
                            error_expr::ExprId df = arena.makeConst(1.0);
                            error_expr::ExprId u = arena.makeEpsilon(node.prec);
                            error_expr::ExprId ez = arena.makeAdd(arena.makeMul(df, err.at(n.src)),
                                                      arena.makeMul(arena.makeMul(vz, arena.makeErrVar(errSym(node.id))), u));
                            return {vz, ez};
                        },

                        [&](const graph::SinhNode& n) -> std::pair<error_expr::ExprId, error_expr::ExprId> {
                            std::string nodeName = "v_" + std::to_string(node.id);
                            error_expr::ExprId vz = arena.add(error_expr::EVarExpr {nodeName});
                            // d/dx[sinh(x)] = cosh(x)
                            error_expr::ExprId df = vz;  // Use result as derivative approximation
                            error_expr::ExprId u = arena.makeEpsilon(node.prec);
                            error_expr::ExprId ez = arena.makeAdd(arena.makeMul(df, err.at(n.src)),
                                                      arena.makeMul(arena.makeMul(vz, arena.makeErrVar(errSym(node.id))), u));
                            return {vz, ez};
                        },

                        [&](const graph::CoshNode& n) -> std::pair<error_expr::ExprId, error_expr::ExprId> {
                            std::string nodeName = "v_" + std::to_string(node.id);
                            error_expr::ExprId vz = arena.add(error_expr::EVarExpr {nodeName});
                            // d/dx[cosh(x)] = sinh(x)
                            error_expr::ExprId df = vz;  // Use result as derivative approximation
                            error_expr::ExprId u = arena.makeEpsilon(node.prec);
                            error_expr::ExprId ez = arena.makeAdd(arena.makeMul(df, err.at(n.src)),
                                                      arena.makeMul(arena.makeMul(vz, arena.makeErrVar(errSym(node.id))), u));
                            return {vz, ez};
                        },

                        [&](const graph::TanhNode& n) -> std::pair<error_expr::ExprId, error_expr::ExprId> {
                            std::string nodeName = "v_" + std::to_string(node.id);
                            error_expr::ExprId vz = arena.add(error_expr::EVarExpr {nodeName});
                            // d/dx[tanh(x)] = 1/cosh^2(x), bounded by (0, 1]
                            error_expr::ExprId df = arena.makeConst(1.0);
                            error_expr::ExprId u = arena.makeEpsilon(node.prec);
                            error_expr::ExprId ez = arena.makeAdd(arena.makeMul(df, err.at(n.src)),
                                                      arena.makeMul(arena.makeMul(vz, arena.makeErrVar(errSym(node.id))), u));
                            return {vz, ez};
                        },

                        [&](const auto&) -> std::pair<error_expr::ExprId, error_expr::ExprId> {
                            throw std::runtime_error("applyRule: unhandled node kind");
                        },

                },
                node.kind);
    }
}  // namespace autodiff::detail
