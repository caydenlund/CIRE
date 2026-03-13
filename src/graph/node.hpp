#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace graph {
    using NodeId = std::uint32_t;

    enum class FloatPrec : std::uint8_t {
        F16,
        BF16,
        F32,
        F64,
        F128,
    };

    /// Returns unit roundoff u such that |fl(x) - x| <= u * |x|
    double unitRoundoff(FloatPrec p);

    /// Returns the mantissa bit-width of a precision (used for cast direction)
    int precBits(FloatPrec p);

    struct Shape {
        std::vector<std::size_t> dims;  // empty == scalar

        static Shape scalar() { return {}; }
        bool isScalar() const { return dims.empty(); }
        std::size_t numElements() const;
    };

    struct DebugLoc {
        std::string file;
        uint32_t line {0};
        uint32_t col {0};
    };

    // ---- leaf kinds ----
    struct InputVarNode {
        std::string name;
    };
    struct ConstantNode {
        double value;
        FloatPrec prec;
    };

    // ---- binary arithmetic ----
    struct AddNode {
        NodeId lhs, rhs;
    };
    struct SubNode {
        NodeId lhs, rhs;
    };
    struct MulNode {
        NodeId lhs, rhs;
    };
    struct DivNode {
        NodeId lhs, rhs;
    };
    struct PowNode {
        NodeId lhs, rhs;
    };

    // ---- unary ----
    struct NegNode {
        NodeId src;
    };
    struct SqrtNode {
        NodeId src;
    };
    struct AbsNode {
        NodeId src;
    };
    struct SinNode {
        NodeId src;
    };
    struct CosNode {
        NodeId src;
    };
    struct TanNode {
        NodeId src;
    };
    struct AsinNode {
        NodeId src;
    };
    struct AcosNode {
        NodeId src;
    };
    struct AtanNode {
        NodeId src;
    };
    struct SinhNode {
        NodeId src;
    };
    struct CoshNode {
        NodeId src;
    };
    struct TanhNode {
        NodeId src;
    };
    struct ExpNode {
        NodeId src;
    };
    struct LogNode {
        NodeId src;
    };

    // ---- special ----
    struct CastNode {
        NodeId src;
        FloatPrec from;
        FloatPrec to;
    };
    struct FmaNode {
        NodeId a, b, c;
    };
    struct ReduceSumNode {
        NodeId src;
        std::optional<std::size_t> axis;
    };

    using NodeKind
            = std::variant<InputVarNode, ConstantNode, AddNode, SubNode, MulNode, DivNode, PowNode, NegNode, SqrtNode,
                           AbsNode, SinNode, CosNode, TanNode, AsinNode, AcosNode, AtanNode, SinhNode, CoshNode, TanhNode,
                           ExpNode, LogNode, CastNode, FmaNode, ReduceSumNode>;

    struct Node {
        NodeId id;
        NodeKind kind;
        FloatPrec prec;
        Shape shape;
        std::optional<DebugLoc> loc;
        std::string llvmName;  // Original LLVM instruction name (e.g., "%mul", "%add")
        std::string llvmIR;    // Full LLVM IR string (e.g., "%mul = fmul double %x, %x")
    };

    /// Utility for std::visit with multiple lambdas
    template<class... TsType>
    struct Overloaded : TsType... {
        using TsType::operator()...;
    };
    template<class... TsType>
    Overloaded(TsType...) -> Overloaded<TsType...>;
}  // namespace graph
