#ifndef CIRE_NODE_H
#define CIRE_NODE_H

#include "ibex_Expr.h"
#include "cire/core/InstructionMetadata.h"

#include <cmath>
#include <cstdint>
#include <map>
#include <memory>
#include <set>

enum NodeType : std::uint8_t {
    DEFAULT,        // Default node type
    INTEGER,        // Represents integers
    FLOAT,          // Represents single precision floating point numbers TODO: Remove FLOAT if not using
    DOUBLE,         // Represents double precision floating point numbers
    FREE_VARIABLE,  // For Input values
    VARIABLE,       // For variables in expressions
    UNARY_OP,       // For unary operations
    BINARY_OP,      // For binary operations
    TERNARY_OP,     // For ternary operations
};

class Node {
public:
    enum Op : std::uint8_t {
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
    std::map<Op, double> opErrorULPs = {
            {ADD, 1.0},  {SUB, 1.0},  {MUL, 1.0},  {DIV, 2.0},     {SIN, 2.0},   {COS, 2.0},  {TAN, 2.0},
            {SINH, 2.0}, {COSH, 2.0}, {TANH, 2.0}, {ASIN, 2.0},    {ACOS, 2.0},  {ATAN, 2.0}, {LOG, 2.0},
            {SQRT, 1.0}, {EXP, 2.0},  {FMA, 2.0},  {FPTRUNC, 0.0}, {FPEXT, 0.0},
    };

    enum RoundingType : std::uint8_t {
        CONST,
        INT,
        FL16,
        FL32,
        FL64,
    };

    static int NEW_FREE_VARIABLE_COUNTER;
    static int NODE_COUNTER;
    int id = NODE_COUNTER++;
    int depth = 0;
    NodeType type = DEFAULT;
    RoundingType opRoundType = INT;
    // Epsilon value for rounding on applying the operator
    double opRounding = 1.0;  // RoundingAmount[INT] by default (no rounding)
    const ibex::ExprNode* absoluteError = nullptr;
    std::set<Node*> parents;

    // Instruction metadata for tracking LLVM IR source
    std::unique_ptr<InstructionMetadata> metadata = nullptr;

    // Epsilon values for rounding to be applied for different types
    std::map<RoundingType, double> roundingAmount = {
            {CONST, 0.0}, {INT, 1.0}, {FL16, pow(2, -11 + 53)}, {FL32, pow(2, -24 + 53)}, {FL64, 1.0},
    };


    Node() = default;
    virtual ~Node() = default;

    bool isInteger() const;
    bool isFloat() const;
    bool isDouble() const;
    bool isFreeVariable() const;
    bool isVariable() const;
    bool isUnaryOp() const;
    bool isBinaryOp() const;
    bool isTernaryOp() const;

    void setRoundingType(RoundingType type);
    void setRoundingFromType(RoundingType type);
    void setRounding(double opRounding);
    void setAbsoluteError(const ibex::ExprNode* absErr);

    RoundingType getRoundingType() const;

    // Instruction metadata methods
    void setMetadata(std::unique_ptr<InstructionMetadata> meta);
    InstructionMetadata* getMetadata() const;
    bool hasMetadata() const;

    virtual void write(std::ostream& os) const;

    virtual ibex::ExprNode* getExprNode() const;

    // Prints string representation of this node
    friend std::ostream& operator<<(std::ostream& os, const Node& node);

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
    virtual Node* getChildNode(int index) const;
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
    ibex::ExprNode* getExprNode() const override;
    bool operator==(const Integer& other) const;
    Node& operator+(Node& other) const override;
    Node& operator-(Node& other) const override;
    Node& operator*(Node& other) const override;
    Node& operator/(Node& other) const override;
    ibex::ExprNode& generateSymExpr() override;
    Node* getChildNode(int index) const override;
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
    ibex::ExprNode* getExprNode() const override;
    bool operator==(const Float& other) const;
    Node& operator+(Node& other) const override;
    Node& operator-(Node& other) const override;
    Node& operator*(Node& other) const override;
    Node& operator/(Node& other) const override;
    ibex::ExprNode& getAbsoluteError() override;
    ibex::ExprNode& generateSymExpr() override;
    Node* getChildNode(int index) const override;
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
    ibex::ExprNode* getExprNode() const override;
    bool operator==(const Double& other) const;
    Node& operator+(Node& other) const override;
    Node& operator-(Node& other) const override;
    Node& operator*(Node& other) const override;
    Node& operator/(Node& other) const override;
    ibex::ExprNode& getAbsoluteError() override;
    ibex::ExprNode& generateSymExpr() override;
    Node* getChildNode(int index) const override;
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
    Node* getChildNode(int index) const override;
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
    ibex::ExprNode* getExprNode() const override;
    bool operator==(const VariableNode& other) const;
    Node& operator+(Node& other) const override;
    Node& operator-(Node& other) const override;
    Node& operator*(Node& other) const override;
    Node& operator/(Node& other) const override;
    ibex::ExprNode& getAbsoluteError() override;
    ibex::ExprNode& generateSymExpr() override;
    Node* getChildNode(int index) const override;
};

class UnaryOp : public Node {
public:
    Node* operand {};
    Op op {};
    const ibex::ExprUnaryOp* expr {};
    UnaryOp() = default;
    UnaryOp(Node* Operand, Op op, RoundingType rnd_typ);
    ~UnaryOp() override = default;

    // Prints string representation of this node
    void write(std::ostream& os) const override;
    ibex::ExprNode* getExprNode() const override;
    bool operator==(const UnaryOp& other) const;
    Node& operator+(Node& other) const override;
    Node& operator-(Node& other) const override;
    Node& operator*(Node& other) const override;
    Node& operator/(Node& other) const override;
    double getRounding() override;
    ibex::ExprNode& generateSymExpr() override;
    Node* getChildNode(int index) const override;
};

class BinaryOp : public Node {
public:
    Node* leftOperand {};
    Node* rightOperand {};
    Op op {};
    const ibex::ExprBinaryOp* expr {};

    BinaryOp() = default;
    BinaryOp(Node* leftOperand, Node* rightOperand, Op op);
    ~BinaryOp() override = default;

    // Prints string representation of this node
    void write(std::ostream& os) const override;
    ibex::ExprNode* getExprNode() const override;
    bool operator==(const BinaryOp& other) const;
    Node& operator+(Node& other) const override;
    Node& operator-(Node& other) const override;
    Node& operator*(Node& other) const override;
    Node& operator/(Node& other) const override;
    double getRounding() override;
    ibex::ExprNode& generateSymExpr() override;
    Node* getChildNode(int index) const override;
};

class TernaryOp : public Node {
private:
public:
    Node* leftOperand {};
    Node* middleOperand {};
    Node* rightOperand {};
    Op op {};
    // IBEX does not have a Ternary Expr so we use a Binary Expr corresponding the last op in the
    // Ternary Op
    const ibex::ExprBinaryOp* expr {};
    TernaryOp() = default;
    TernaryOp(Node* leftOperand, Node* middleOperand, Node* rightOperand, Op op);
    ~TernaryOp() override = default;

    // Prints string representation of this node
    void write(std::ostream& os) const override;
    ibex::ExprNode* getExprNode() const override;
    bool operator==(const TernaryOp& other) const;
    Node& operator+(Node& other) const override;
    Node& operator-(Node& other) const override;
    Node& operator*(Node& other) const override;
    Node& operator/(Node& other) const override;
    double getRounding() override;
    ibex::ExprNode& generateSymExpr() override;
    Node* getChildNode(int index) const override;
};

std::ostream& operator<<(std::ostream& os, const Node& node);
Node& fma(Node& x, Node& y, Node& z);
Node& operator+(Node& x, Node* y);
Node& operator-(Node& x, Node* y);
Node& operator*(Node& x, Node* y);
Node& operator/(Node& x, Node* y);
Node& operator-(Node& x);
Node& sin(Node& x);
Node& cos(Node& x);
Node& tan(Node& x);
Node& sinh(Node& x);
Node& cosh(Node& x);
Node& tanh(Node& x);
Node& asin(Node& x);
Node& acos(Node& x);
Node& atan(Node& x);
Node& log(Node& x);
Node& sqrt(Node& x);
Node& exp(Node& x);
Node& fptrunc(Node& x, Node::RoundingType rnd_typ);
Node& fpext(Node& x, Node::RoundingType rnd_typ);

const ibex::ExprNode& product(const ibex::ExprNode& left, const ibex::ExprNode& right);
const ibex::ExprNode& product(const ibex::ExprNode& left, double right);

#endif  // CIRE_NODE_H
