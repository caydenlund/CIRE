#pragma once

#include "graph/node.hpp"

#include <cstdint>
#include <iosfwd>
#include <set>
#include <string>
#include <variant>
#include <vector>

namespace error_expr {
    using ExprId = std::uint32_t;

    // ---- leaves ----
    struct EVarExpr {
        std::string name;
    };  // input variable x_i
    struct EErrVar {
        std::string name;
    };  // error symbol ε_i
    struct EConst {
        double value;
    };
    struct EEpsilon {
        graph::FloatPrec prec;
    };  // unit roundoff u(prec)

    // ---- compound ----
    struct EAdd {
        ExprId lhs, rhs;
    };
    struct ESub {
        ExprId lhs, rhs;
    };
    struct EMul {
        ExprId lhs, rhs;
    };
    struct EDiv {
        ExprId lhs, rhs;
    };
    struct ENeg {
        ExprId src;
    };
    struct EAbs {
        ExprId src;
    };
    struct EPow {
        ExprId base;
        int exp;
    };

    using ExprKind = std::variant<EVarExpr, EErrVar, EConst, EEpsilon, EAdd, ESub, EMul, EDiv, ENeg, EAbs, EPow>;

    /// Arena-allocated symbolic expression tree.
    class ErrorExpr {
    public:
        [[nodiscard]] ExprId add(ExprKind kind);

        [[nodiscard]] ExprId makeVar(std::string name);
        [[nodiscard]] ExprId makeErrVar(std::string name);
        [[nodiscard]] ExprId makeConst(double v);
        [[nodiscard]] ExprId makeEpsilon(graph::FloatPrec p);
        [[nodiscard]] ExprId makeAdd(ExprId a, ExprId b);
        [[nodiscard]] ExprId makeSub(ExprId a, ExprId b);
        [[nodiscard]] ExprId makeMul(ExprId a, ExprId b);
        [[nodiscard]] ExprId makeNeg(ExprId a);
        [[nodiscard]] ExprId makeAbs(ExprId a);

        const ExprKind& get(ExprId id) const { return _nodes.at(id); }
        ExprId root() const { return _root; }
        void setRoot(ExprId id) { _root = id; }

        std::set<std::string> freeVars() const;
        std::set<std::string> errorVars() const;

        std::string toSollyaString() const;
        std::string toDRealSMT2() const;

        void dumpAST(std::ostream& os) const;

    private:
        std::vector<ExprKind> _nodes;
        ExprId _root {0};
    };
}  // namespace error_expr
