#ifndef CIRE_IR_NODE
#define CIRE_IR_NODE

#include "cire/core/InstructionMetadata.h"
#include "ibex_Expr.h"

#include <cmath>
#include <cstdint>
#include <map>
#include <memory>
#include <set>

namespace ir {
    class Node {
    public:
        enum class Type : std::uint8_t {
            DEFAULT,        // Default node type
            INTEGER,        // Integers
            FLOAT,          // Single precision floating point numbers
            DOUBLE,         // Double precision floating point numbers
            FREE_VARIABLE,  // Input values
            VARIABLE,       // Variables in expressions
            UNARY_OP,       // Unary operations
            BINARY_OP,      // Binary operations
            TERNARY_OP,     // Ternary operations
        };

        enum class OpType : std::uint8_t {
            ADD,
            SUB,
            MUL,
            DIV,
            NEG,
            SIN,
            COS,
            TAN,
            SINH,
            COSH,
            TANH,
            ASIN,
            ACOS,
            ATAN,
            LOG,
            SQRT,
            EXP,
            FMA,
            FPTRUNC,
            FPEXT,
        };

        // The amount of error on these operations
        std::map<OpType, double> opErrorULPs = {
                {OpType::ADD, 1.0},  {OpType::SUB, 1.0},     {OpType::MUL, 1.0},   {OpType::DIV, 2.0},
                {OpType::SIN, 2.0},  {OpType::COS, 2.0},     {OpType::TAN, 2.0},   {OpType::SINH, 2.0},
                {OpType::COSH, 2.0}, {OpType::TANH, 2.0},    {OpType::ASIN, 2.0},  {OpType::ACOS, 2.0},
                {OpType::ATAN, 2.0}, {OpType::LOG, 2.0},     {OpType::SQRT, 1.0},  {OpType::EXP, 2.0},
                {OpType::FMA, 1.0},  {OpType::FPTRUNC, 0.0}, {OpType::FPEXT, 0.0},
        };

        enum class RoundingType : std::uint8_t {
            CONST,
            INT,
            FL16,
            FL32,
            FL64,
        };

        static int NEW_FREE_VARIABLE_COUNTER;
        static int NODE_COUNTER;
        unsigned id = NODE_COUNTER++;
        unsigned depth {0};
        Type type {Type::DEFAULT};
        RoundingType opRoundType {RoundingType::INT};
        // Epsilon value for rounding on applying the operator
        double opRounding = 1.0;  // RoundingAmount[INT] by default (no rounding)
        const ibex::ExprNode* absoluteError {};
        std::set<Node*> parents;

        // Instruction metadata for tracking LLVM IR source
        std::unique_ptr<InstructionMetadata> metadata {nullptr};

        // Epsilon values for rounding to be applied for different types
        std::map<RoundingType, double> roundingAmount = {
                {RoundingType::CONST, 0.0},
                {RoundingType::INT, 1.0},
                {RoundingType::FL16, pow(2, -11 + 53)},
                {RoundingType::FL32, pow(2, -24 + 53)},
                {RoundingType::FL64, 1.0},
        };

        Node() = default;
        virtual ~Node() = default;

        [[nodiscard]] bool isInteger() const;
        [[nodiscard]] bool isFloat() const;
        [[nodiscard]] bool isDouble() const;
        [[nodiscard]] bool isFreeVariable() const;
        [[nodiscard]] bool isVariable() const;
        [[nodiscard]] bool isUnaryOp() const;
        [[nodiscard]] bool isBinaryOp() const;
        [[nodiscard]] bool isTernaryOp() const;

        void setRoundingType(RoundingType type);
        void setRoundingFromType(RoundingType type);
        void setRounding(double opRounding);
        void setAbsoluteError(const ibex::ExprNode* absErr);

        [[nodiscard]] RoundingType getRoundingType() const;

        // Instruction metadata methods
        void setMetadata(std::unique_ptr<InstructionMetadata> meta);
        [[nodiscard]] InstructionMetadata* getMetadata() const;
        [[nodiscard]] bool hasMetadata() const;

        virtual void write(std::ostream& os) const;

        [[nodiscard]] virtual ibex::ExprNode* getExprNode() const;

        bool operator==(const Node& other) const;
        friend Node& operator+(Node& x, Node* y);
        friend Node& operator-(Node& x, Node* y);
        friend Node& operator*(Node& x, Node* y);
        friend Node& operator/(Node& x, Node* y);
        virtual Node& operator+(Node& other) const;
        virtual Node& operator-(Node& other) const;
        virtual Node& operator*(Node& other) const;
        virtual Node& operator/(Node& other) const;
        virtual double getRounding();
        virtual ibex::ExprNode& getAbsoluteError();
        virtual ibex::ExprNode& generateSymExpr();
        [[nodiscard]] virtual Node* getChildNode(int index) const;
    };

    class Integer : public Node {
    public:
        const ibex::ExprConstant* value = nullptr;
        const int val = 0;
        Integer() = default;
        explicit Integer(int);
        explicit Integer(const ibex::ExprConstant& value);
        ~Integer() override = default;

        // Prints string representation of this node
        void write(std::ostream& os) const override;
        [[nodiscard]] ibex::ExprNode* getExprNode() const override;
        bool operator==(const Integer& other) const;
        Node& operator+(Node& other) const override;
        Node& operator-(Node& other) const override;
        Node& operator*(Node& other) const override;
        Node& operator/(Node& other) const override;
        ibex::ExprNode& generateSymExpr() override;
        [[nodiscard]] Node* getChildNode(int index) const override;
    };

    class Float : public Node {
    public:
        const ibex::ExprConstant* value = nullptr;
        const float val = 0.0;
        Float() = default;
        explicit Float(float);
        explicit Float(const ibex::ExprConstant& value);
        ~Float() override = default;

        // Prints string representation of this node
        void write(std::ostream& os) const override;
        [[nodiscard]] ibex::ExprNode* getExprNode() const override;
        bool operator==(const Float& other) const;
        Node& operator+(Node& other) const override;
        Node& operator-(Node& other) const override;
        Node& operator*(Node& other) const override;
        Node& operator/(Node& other) const override;
        ibex::ExprNode& getAbsoluteError() override;
        ibex::ExprNode& generateSymExpr() override;
        [[nodiscard]] Node* getChildNode(int index) const override;
    };

    class Double : public Node {
    public:
        const ibex::ExprConstant* value = nullptr;
        const double val = 0.0;
        Double() = default;
        explicit Double(double);
        explicit Double(const ibex::ExprConstant& value);
        ~Double() override = default;

        // Prints string representation of this node
        void write(std::ostream& os) const override;
        [[nodiscard]] ibex::ExprNode* getExprNode() const override;
        bool operator==(const Double& other) const;
        Node& operator+(Node& other) const override;
        Node& operator-(Node& other) const override;
        Node& operator*(Node& other) const override;
        Node& operator/(Node& other) const override;
        ibex::ExprNode& getAbsoluteError() override;
        ibex::ExprNode& generateSymExpr() override;
        [[nodiscard]] Node* getChildNode(int index) const override;
    };

    // Represents Input Intervals
    class FreeVariable : public Node {
    public:
        const ibex::Interval* var = nullptr;
        FreeVariable();
        explicit FreeVariable(RoundingType rnd_typ);
        explicit FreeVariable(const ibex::Interval& var, RoundingType rnd_typ);
        ~FreeVariable() override = default;

        // Prints string representation of this node
        void write(std::ostream& os) const override;
        bool operator==(const FreeVariable& other) const;
        Node& operator+(Node& other) const override;
        Node& operator-(Node& other) const override;
        Node& operator*(Node& other) const override;
        Node& operator/(Node& other) const override;
        ibex::ExprNode& getAbsoluteError() override;
        ibex::ExprNode& generateSymExpr() override;
        [[nodiscard]] Node* getChildNode(int index) const override;
    };

    // Represents interval assigned variables
    class VariableNode : public Node {
    public:
        const ibex::ExprSymbol* variable;
        VariableNode();
        explicit VariableNode(RoundingType rnd_typ);
        explicit VariableNode(const ibex::ExprSymbol& variable);
        explicit VariableNode(const Node& node);
        ~VariableNode() override = default;

        // Prints string representation of this node
        void write(std::ostream& os) const override;
        [[nodiscard]] ibex::ExprNode* getExprNode() const override;
        bool operator==(const VariableNode& other) const;
        Node& operator+(Node& other) const override;
        Node& operator-(Node& other) const override;
        Node& operator*(Node& other) const override;
        Node& operator/(Node& other) const override;
        ibex::ExprNode& getAbsoluteError() override;
        ibex::ExprNode& generateSymExpr() override;
        [[nodiscard]] Node* getChildNode(int index) const override;
    };

    class UnaryOp : public Node {
    public:
        Node* operand {};
        Node::OpType op {};
        const ibex::ExprUnaryOp* expr {};
        UnaryOp() = default;
        UnaryOp(Node* Operand, Node::OpType op, RoundingType rnd_typ);
        ~UnaryOp() override = default;

        // Prints string representation of this node
        void write(std::ostream& os) const override;
        [[nodiscard]] ibex::ExprNode* getExprNode() const override;
        bool operator==(const UnaryOp& other) const;
        Node& operator+(Node& other) const override;
        Node& operator-(Node& other) const override;
        Node& operator*(Node& other) const override;
        Node& operator/(Node& other) const override;
        double getRounding() override;
        ibex::ExprNode& generateSymExpr() override;
        [[nodiscard]] Node* getChildNode(int index) const override;
    };

    class BinaryOp : public Node {
    public:
        Node* leftOperand {};
        Node* rightOperand {};
        Node::OpType op {};
        const ibex::ExprBinaryOp* expr {};

        BinaryOp() = default;
        BinaryOp(Node* leftOperand, Node* rightOperand, Node::OpType op);
        ~BinaryOp() override = default;

        // Prints string representation of this node
        void write(std::ostream& os) const override;
        [[nodiscard]] ibex::ExprNode* getExprNode() const override;
        bool operator==(const BinaryOp& other) const;
        Node& operator+(Node& other) const override;
        Node& operator-(Node& other) const override;
        Node& operator*(Node& other) const override;
        Node& operator/(Node& other) const override;
        double getRounding() override;
        ibex::ExprNode& generateSymExpr() override;
        [[nodiscard]] Node* getChildNode(int index) const override;
    };

    class TernaryOp : public Node {
    private:
    public:
        Node* leftOperand {};
        Node* middleOperand {};
        Node* rightOperand {};
        Node::OpType op {};
        // IBEX does not have a Ternary Expr so we use a Binary Expr corresponding the last op in the
        // Ternary Op
        const ibex::ExprBinaryOp* expr {};
        TernaryOp() = default;
        TernaryOp(Node* leftOperand, Node* middleOperand, Node* rightOperand, Node::OpType op);
        ~TernaryOp() override = default;

        // Prints string representation of this node
        void write(std::ostream& os) const override;
        [[nodiscard]] ibex::ExprNode* getExprNode() const override;
        bool operator==(const TernaryOp& other) const;
        Node& operator+(Node& other) const override;
        Node& operator-(Node& other) const override;
        Node& operator*(Node& other) const override;
        Node& operator/(Node& other) const override;
        double getRounding() override;
        ibex::ExprNode& generateSymExpr() override;
        [[nodiscard]] Node* getChildNode(int index) const override;
    };
}  // namespace ir

std::ostream& operator<<(std::ostream& os, const ir::Node& node);
ir::Node& fma(ir::Node& x, ir::Node& y, ir::Node& z);
ir::Node& operator-(ir::Node& x);
ir::Node& sin(ir::Node& x);
ir::Node& cos(ir::Node& x);
ir::Node& tan(ir::Node& x);
ir::Node& sinh(ir::Node& x);
ir::Node& cosh(ir::Node& x);
ir::Node& tanh(ir::Node& x);
ir::Node& asin(ir::Node& x);
ir::Node& acos(ir::Node& x);
ir::Node& atan(ir::Node& x);
ir::Node& log(ir::Node& x);
ir::Node& sqrt(ir::Node& x);
ir::Node& exp(ir::Node& x);
ir::Node& fptrunc(ir::Node& x, ir::Node::RoundingType rnd_typ);
ir::Node& fpext(ir::Node& x, ir::Node::RoundingType rnd_typ);

const ibex::ExprNode& product(const ibex::ExprNode& left, const ibex::ExprNode& right);
const ibex::ExprNode& product(const ibex::ExprNode& left, double right);

#endif  // CIRE_NODE_H
