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

  // Prints string representation of this node
  friend std::ostream& operator<<(std::ostream& os, const Node &node);
};

class Integer : public Node {
private:

public:
  int value = 0;
  Integer() = default;
  Integer(int value);
  ~Integer() = default;

  // Prints string representation of this node
  void write(std::ostream& os) const override;
};

class Float : public Node {
private:

public:
  float value = 0.0;
  Float() = default;
  Float(float value);
  ~Float() = default;

  // Prints string representation of this node
  void write(std::ostream& os) const override;
};

class Double : public Node {
private:

public:
  double value = 0.0;
  Double() = default;
  Double(double value);
  ~Double() = default;

  // Prints string representation of this node
  void write(std::ostream& os) const override;
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
};

// Represents assigned variables
class VariableNode : public Node {
private:
public:
  string name;
  VariableNode() = default;
  VariableNode(string name);
  ~VariableNode() = default;

  // Prints string representation of this node
  void write(std::ostream& os) const override;
};

class UnaryOp : public Node {
private:
public:
  Node* Operand;
  UnaryOp() = default;
  UnaryOp(Node* Operand);
  ~UnaryOp() = default;

  // Prints string representation of this node
  void write(std::ostream& os) const override;
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
public:
  BinaryOp() = default;
  BinaryOp(Node* leftOperand, Node* rightOperand, Op op);
  ~BinaryOp() = default;

  // Prints string representation of this node
  void write(std::ostream& os) const override;
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
};

std::ostream &operator<<(std::ostream &os, const Node &node);

#endif //CIRE_NODE_H
