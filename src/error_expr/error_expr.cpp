#include "error_expr/error_expr.hpp"
#include "graph/node.hpp"

#include <functional>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace error_expr {

    ExprId ErrorExpr::add(ExprKind kind) {
        ExprId id = static_cast<ExprId>(_nodes.size());
        _nodes.push_back(std::move(kind));
        return id;
    }

    ExprId ErrorExpr::makeVar(std::string name) {
        return add(EVarExpr{std::move(name)});
    }

    ExprId ErrorExpr::makeErrVar(std::string name) {
        return add(EErrVar{std::move(name)});
    }

    ExprId ErrorExpr::makeConst(double v) {
        return add(EConst{v});
    }

    ExprId ErrorExpr::makeEpsilon(graph::FloatPrec p) {
        return add(EEpsilon{p});
    }

    ExprId ErrorExpr::makeAdd(ExprId a, ExprId b) {
        return add(EAdd{a, b});
    }

    ExprId ErrorExpr::makeSub(ExprId a, ExprId b) {
        return add(ESub{a, b});
    }

    ExprId ErrorExpr::makeMul(ExprId a, ExprId b) {
        return add(EMul{a, b});
    }

    ExprId ErrorExpr::makeNeg(ExprId a) {
        return add(ENeg{a});
    }

    ExprId ErrorExpr::makeAbs(ExprId a) {
        return add(EAbs{a});
    }

    std::set<std::string> ErrorExpr::freeVars() const {
        std::set<std::string> vars;

        for (const auto& node : _nodes) {
            std::visit([&](auto&& kind) {
                using T = std::decay_t<decltype(kind)>;
                if constexpr (std::is_same_v<T, EVarExpr>) {
                    vars.insert(kind.name);
                }
            }, node);
        }

        return vars;
    }

    std::set<std::string> ErrorExpr::errorVars() const {
        std::set<std::string> vars;

        for (const auto& node : _nodes) {
            std::visit([&](auto&& kind) {
                using T = std::decay_t<decltype(kind)>;
                if constexpr (std::is_same_v<T, EErrVar>) {
                    vars.insert(kind.name);
                }
            }, node);
        }

        return vars;
    }

    void ErrorExpr::simplify() {
        // TODO: Implement algebraic simplification
        // For now, this is a no-op
        // Could implement constant folding, identity elimination, etc.
    }

    std::string ErrorExpr::toSollyaString() const {
        // Helper lambda for recursive conversion
        std::function<std::string(ExprId)> convert = [&](ExprId id) -> std::string {
            const ExprKind& kind = get(id);

            return std::visit([&](auto&& node) -> std::string {
                using T = std::decay_t<decltype(node)>;

                if constexpr (std::is_same_v<T, EVarExpr>) {
                    return node.name;
                }
                else if constexpr (std::is_same_v<T, EErrVar>) {
                    return node.name;
                }
                else if constexpr (std::is_same_v<T, EConst>) {
                    std::ostringstream oss;
                    oss << std::scientific << node.value;
                    return oss.str();
                }
                else if constexpr (std::is_same_v<T, EEpsilon>) {
                    double u = graph::unitRoundoff(node.prec);
                    std::ostringstream oss;
                    oss << std::scientific << u;
                    return oss.str();
                }
                else if constexpr (std::is_same_v<T, EAdd>) {
                    return "(" + convert(node.lhs) + " + " + convert(node.rhs) + ")";
                }
                else if constexpr (std::is_same_v<T, ESub>) {
                    return "(" + convert(node.lhs) + " - " + convert(node.rhs) + ")";
                }
                else if constexpr (std::is_same_v<T, EMul>) {
                    return "(" + convert(node.lhs) + " * " + convert(node.rhs) + ")";
                }
                else if constexpr (std::is_same_v<T, EDiv>) {
                    return "(" + convert(node.lhs) + " / " + convert(node.rhs) + ")";
                }
                else if constexpr (std::is_same_v<T, ENeg>) {
                    return "(-" + convert(node.src) + ")";
                }
                else if constexpr (std::is_same_v<T, EAbs>) {
                    return "abs(" + convert(node.src) + ")";
                }
                else if constexpr (std::is_same_v<T, EPow>) {
                    return "(" + convert(node.base) + ")^" + std::to_string(node.exp);
                }
                else {
                    throw std::runtime_error("Unknown expression type in toSollyaString");
                }
            }, kind);
        };

        return convert(_root);
    }

    std::string ErrorExpr::toDRealSMT2() const {
        // Helper lambda for recursive conversion
        std::function<std::string(ExprId)> convert = [&](ExprId id) -> std::string {
            const ExprKind& kind = get(id);

            return std::visit([&](auto&& node) -> std::string {
                using T = std::decay_t<decltype(node)>;

                if constexpr (std::is_same_v<T, EVarExpr>) {
                    return node.name;
                }
                else if constexpr (std::is_same_v<T, EErrVar>) {
                    return node.name;
                }
                else if constexpr (std::is_same_v<T, EConst>) {
                    std::ostringstream oss;
                    oss << std::scientific << node.value;
                    return oss.str();
                }
                else if constexpr (std::is_same_v<T, EEpsilon>) {
                    double u = graph::unitRoundoff(node.prec);
                    std::ostringstream oss;
                    oss << std::scientific << u;
                    return oss.str();
                }
                else if constexpr (std::is_same_v<T, EAdd>) {
                    return "(+ " + convert(node.lhs) + " " + convert(node.rhs) + ")";
                }
                else if constexpr (std::is_same_v<T, ESub>) {
                    return "(- " + convert(node.lhs) + " " + convert(node.rhs) + ")";
                }
                else if constexpr (std::is_same_v<T, EMul>) {
                    return "(* " + convert(node.lhs) + " " + convert(node.rhs) + ")";
                }
                else if constexpr (std::is_same_v<T, EDiv>) {
                    return "(/ " + convert(node.lhs) + " " + convert(node.rhs) + ")";
                }
                else if constexpr (std::is_same_v<T, ENeg>) {
                    return "(- " + convert(node.src) + ")";
                }
                else if constexpr (std::is_same_v<T, EAbs>) {
                    return "(abs " + convert(node.src) + ")";
                }
                else if constexpr (std::is_same_v<T, EPow>) {
                    return "(^ " + convert(node.base) + " " + std::to_string(node.exp) + ")";
                }
                else {
                    throw std::runtime_error("Unknown expression type in toDRealSMT2");
                }
            }, kind);
        };

        return convert(_root);
    }

    void ErrorExpr::dumpAST(std::ostream& os) const {
        // Helper lambda for recursive printing with indentation
        std::function<void(ExprId, int)> dump = [&](ExprId id, int indent) {
            auto printIndent = [&]() {
                for (int i = 0; i < indent; ++i) os << "  ";
            };

            const ExprKind& kind = get(id);

            std::visit([&](auto&& node) {
                using T = std::decay_t<decltype(node)>;

                printIndent();
                if constexpr (std::is_same_v<T, EVarExpr>) {
                    os << "Var[" << id << "]: " << node.name << "\n";
                }
                else if constexpr (std::is_same_v<T, EErrVar>) {
                    os << "ErrVar[" << id << "]: " << node.name << "\n";
                }
                else if constexpr (std::is_same_v<T, EConst>) {
                    os << "Const[" << id << "]: " << node.value << "\n";
                }
                else if constexpr (std::is_same_v<T, EEpsilon>) {
                    os << "Epsilon[" << id << "]: u(";
                    switch (node.prec) {
                        case graph::FloatPrec::F16:   os << "F16"; break;
                        case graph::FloatPrec::BF16:  os << "BF16"; break;
                        case graph::FloatPrec::F32:   os << "F32"; break;
                        case graph::FloatPrec::F64:   os << "F64"; break;
                        case graph::FloatPrec::F128:  os << "F128"; break;
                    }
                    os << ") = " << graph::unitRoundoff(node.prec) << "\n";
                }
                else if constexpr (std::is_same_v<T, EAdd>) {
                    os << "Add[" << id << "]:\n";
                    dump(node.lhs, indent + 1);
                    dump(node.rhs, indent + 1);
                }
                else if constexpr (std::is_same_v<T, ESub>) {
                    os << "Sub[" << id << "]:\n";
                    dump(node.lhs, indent + 1);
                    dump(node.rhs, indent + 1);
                }
                else if constexpr (std::is_same_v<T, EMul>) {
                    os << "Mul[" << id << "]:\n";
                    dump(node.lhs, indent + 1);
                    dump(node.rhs, indent + 1);
                }
                else if constexpr (std::is_same_v<T, EDiv>) {
                    os << "Div[" << id << "]:\n";
                    dump(node.lhs, indent + 1);
                    dump(node.rhs, indent + 1);
                }
                else if constexpr (std::is_same_v<T, ENeg>) {
                    os << "Neg[" << id << "]:\n";
                    dump(node.src, indent + 1);
                }
                else if constexpr (std::is_same_v<T, EAbs>) {
                    os << "Abs[" << id << "]:\n";
                    dump(node.src, indent + 1);
                }
                else if constexpr (std::is_same_v<T, EPow>) {
                    os << "Pow[" << id << "]^" << node.exp << ":\n";
                    dump(node.base, indent + 1);
                }
            }, kind);
        };

        os << "ErrorExpr AST (root=" << _root << "):\n";
        dump(_root, 0);
    }

}  // namespace error_expr
