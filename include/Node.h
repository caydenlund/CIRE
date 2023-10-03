#include "ibex.h"
#include<map>
#include<cmath>

#ifndef CIRE_NODE_H
#define CIRE_NODE_H

enum NodeType {
  DEFAULT,        // Default node type
  INTEGER,         // Represents integers
  FLOAT,          // Represents single precision floating point numbers
  DOUBLE,         // Represents double precision floating point numbers
  FREE_VARIABLE,  // For Input variables
  VARIABLE,       // For variables in expressions
  UNARY_OP,       // For unary operations
  BINARY_OP,      // For binary operations
  TERNARY_OP,     // For ternary operations
};

class Node {
public:

  enum RoundingType {
    CONST,
    INT,
    FL32,
    FL64,
  };

protected:


public:
  static int NODE_COUNTER;
  int id = NODE_COUNTER++;
  int depth = 0;
  NodeType type = DEFAULT;
  RoundingType rounding = INT;

// The amount of rounding to be applied
  std::map<RoundingType, double> RoundingAmount = {
    {CONST, 0.0},
    {INT, 0.0},
    {FL32, pow(2, -24+53)},
    {FL64, 1.0},};


  Node() = default;
  ~Node() = default;

  virtual void write(std::ostream &os) const;

  virtual ibex::ExprNode *getExprNode() const;

  // Prints string representation of this node
  friend std::ostream& operator<<(std::ostream& os, const Node &node);

  bool operator==(const Node &other) const;
};

class Integer : public Node {
private:

public:
  const ibex::ExprConstant *value = nullptr;
  Integer() = default;
  Integer(const ibex::ExprConstant &value);
  ~Integer() = default;

  // Prints string representation of this node
  void write(std::ostream& os) const override;
  ibex::ExprNode *getExprNode() const override;
  bool operator==(const Integer &other) const;
};

class Float : public Node {
private:

public:
  const ibex::ExprConstant *value = nullptr;
  Float() = default;
  Float(const ibex::ExprConstant &value);
  ~Float() = default;

  // Prints string representation of this node
  void write(std::ostream& os) const override;
  ibex::ExprNode *getExprNode() const override;
  bool operator==(const Float &other) const;
};

class Double : public Node {
private:

public:
  const ibex::ExprConstant *value = nullptr;
  Double() = default;
  Double(const ibex::ExprConstant &value);
  ~Double() = default;

  // Prints string representation of this node
  void write(std::ostream& os) const override;
  ibex::ExprNode *getExprNode() const override;
  bool operator==(const Double &other) const;
};

// Represents Input variables
class FreeVariable : public Node {
private:

public:
  ibex::Interval var;
  FreeVariable() = default;
  FreeVariable(ibex::Interval var);
  ~FreeVariable() = default;

  // Prints string representation of this node
  void write(std::ostream& os) const override;
  bool operator==(const FreeVariable &other) const;
};

// Represents assigned variables
class VariableNode : public Node {
private:
public:
  const ibex::ExprSymbol *variable ;
  VariableNode() = default;
  VariableNode(const ibex::ExprSymbol& name);
  ~VariableNode() = default;

  // Prints string representation of this node
  void write(std::ostream& os) const override;
  ibex::ExprNode *getExprNode() const override;
  bool operator==(const VariableNode &other) const;
};

class UnaryOp : public Node {
private:
public:
  enum Op {
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
  };
  Node* Operand;
  Op op;
  const ibex::ExprUnaryOp* expr;
  UnaryOp() = default;
  UnaryOp(Node* Operand, Op op, const ibex::ExprUnaryOp &expr);
  ~UnaryOp() = default;

  // Prints string representation of this node
  void write(std::ostream& os) const override;
  bool operator==(const UnaryOp &other) const;
};

class BinaryOp : public Node {
public:
  enum Op {
    ADD,
    SUB,
    MUL,
    DIV,
  };
//private:
  Node* leftOperand;
  Node* rightOperand;
  Op op;
  const ibex::ExprBinaryOp* expr;
public:
  BinaryOp() = default;
  BinaryOp(Node* leftOperand, Node* rightOperand, Op op, const ibex::ExprBinaryOp &expr);
  ~BinaryOp() = default;

  // Prints string representation of this node
  void write(std::ostream& os) const override;
  bool operator==(const BinaryOp &other) const;
};

class TernaryOp : public Node {
private:

public:
  Node* leftOperand;
  Node* middleOperand;
  Node* rightOperand;
  TernaryOp() = default;
  TernaryOp(Node* leftOperand, Node* middleOperand, Node* rightOperand);
  ~TernaryOp() = default;

  // Prints string representation of this node
  void write(std::ostream& os) const override;
  bool operator==(const TernaryOp &other) const;
};

std::ostream &operator<<(std::ostream &os, const Node &node);

#endif //CIRE_NODE_H
