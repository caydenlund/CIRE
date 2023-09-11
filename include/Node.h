#include "Operations.h"
#include "ibex.h"

using namespace ibex;

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
private:
  int id = 0;
  int depth = 0;
  NodeType type = DEFAULT;
  RoundingType rounding = INT;

public:
  Node() = default;
  ~Node() = default;
};

class Integer : public Node {
private:
  int value = 0;
public:
  Integer() = default;
  ~Integer() = default;
};

class Float : public Node {
private:
  float value = 0.0;
public:
  Float() = default;
  ~Float() = default;
};

class Double : public Node {
private:
  double value = 0.0;
public:
  Double() = default;
  ~Double() = default;
};

// Represents Input variables
class FreeVariable : public Node {
private:
  Interval var;
public:
  FreeVariable() = default;
  ~FreeVariable() = default;
};

// Represents assigned variables
class VariableNode : public Node {
private:
  string name;
public:
  VariableNode() = default;
  ~VariableNode() = default;
};

class UnaryOp : public Node {
private:
  Node* Operand;
public:
  UnaryOp() = default;
  ~UnaryOp() = default;
};

class BinaryOp : public Node {
private:
  Node* leftOperand;
  Node* rightOperand;
public:
  BinaryOp() = default;
  ~BinaryOp() = default;
};

class TernaryOp : public Node {
private:
  Node* leftOperand;
  Node* middleOperand;
  Node* rightOperand;
public:
  TernaryOp() = default;
  ~TernaryOp() = default;
};

#endif //CIRE_NODE_H
