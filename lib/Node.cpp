#include "../include/Node.h"

#include <iostream>

int Node::NODE_COUNTER = 0;

std::ostream &operator<<(std::ostream &os, const Node &node) {
  node.write(os);
  return os;
}

void Node::write(std::ostream &os) const {
  os << "\nID:" << id << std::endl;
  os << "\tDepth:" << depth << std::endl;

  // Print type
  std::string type_string = "";
  switch(type) {
    case INTEGER: type_string = "INTEGER"; break;
    case FLOAT: type_string = "FLOAT"; break;
    case DOUBLE: type_string = "DOUBLE"; break;
    case FREE_VARIABLE: type_string = "FREE_VARIABLE"; break;
    case VARIABLE: type_string = "VARIABLE"; break;
    case UNARY_OP: type_string = "UNARY_OP"; break;
    case BINARY_OP: type_string = "BINARY_OP"; break;
    case TERNARY_OP: type_string = "TERNARY_OP"; break;
    default: type_string = "DEFAULT"; break;
  }
  os << "\tType:" << type_string << std::endl;
  os << "\tRounding:" << rounding << std::endl;
}

ibex::ExprNode *Node::getExprNode() const {

}

Integer::Integer(const ibex::ExprConstant &value) {
  this->value = &value;
  this->type = INTEGER;
}

void Integer::write(std::ostream &os) const {
  // Call parent class operator
  Node::write(os);

  // Print remaining data
  os << "\tValue:" << value << std::endl;
}

ibex::ExprNode *Integer::getExprNode() const {
  return (ibex::ExprNode*)this->value;
}

Float::Float(const ibex::ExprConstant &value) {
  this->value = &value;
  this->type = FLOAT;
}

void Float::write(std::ostream &os) const {
  // Call parent class operator
  Node::write(os);

  // Print remaining data
  os << "\tValue:" << value << std::endl;
}

ibex::ExprNode *Float::getExprNode() const {
  return (ibex::ExprNode*)this->value;
}

Double::Double(const ibex::ExprConstant &value) {
  this->value = &value;
  this->type = DOUBLE;
}

void Double::write(std::ostream &os) const {
  // Call parent class operator
  Node::write(os);

  // Print remaining data
  os << "\tValue:" << value << std::endl;
}

ibex::ExprNode *Double::getExprNode() const {
  return (ibex::ExprNode*)this->value;
}

FreeVariable::FreeVariable(ibex::Interval var) {
  this->var = var;
  this->type = FREE_VARIABLE;
}

void FreeVariable::write(std::ostream &os) const {
  // Call parent class operator
  Node::write(os);

  // Print remaining data
  os << "\tValue:" << var << std::endl;
}

VariableNode::VariableNode(const ibex::ExprSymbol& variable) {
  this->variable = &variable;
  this->type = VARIABLE;
}

void VariableNode::write(std::ostream &os) const {
  // Call parent class operator
  Node::write(os);

  // Print remaining data
  os << "\tVariable:" << *variable << std::endl;
}

ibex::ExprNode *VariableNode::getExprNode() const {
  return (ibex::ExprNode*)this->variable;
}

UnaryOp::UnaryOp(Node* Operand, Op op, const ibex::ExprUnaryOp &expr) {
  this->depth = Operand->depth + 1;
  this->type = UNARY_OP;
  this->Operand = Operand;
  this->op = op;
  this->expr = &expr;
}

void UnaryOp::write(std::ostream &os) const {
  // Call parent class operator
  Node::write(os);

  // Print remaining data
  os << "\tOperand: [" << (Node)*Operand << "]" << std::endl;
}

BinaryOp::BinaryOp(Node* Left, Node* Right, Op op, const ibex::ExprBinaryOp &expr) {
  this->depth = std::max(Left->depth, Right->depth) + 1;
  this->type = BINARY_OP;
  this->leftOperand = Left;
  this->rightOperand = Right;
  this->op = op;
  this->expr = &expr;
}

void BinaryOp::write(std::ostream &os) const {
  // Call parent class operator
  Node::write(os);

  // Print remaining data
  os << "\tLeft Operand: [" << (Node)*leftOperand << "]" << std::endl;
  // Print operator
  std::string operator_string = "";
  switch(op) {
    case ADD: operator_string = "+"; break;
    case SUB: operator_string = "-"; break;
    case MUL: operator_string = "*"; break;
    case DIV: operator_string = "/"; break;
    default: operator_string = "Error: Unknown operator accepted"; break;
  }

  os << "\tOperator: " << operator_string << "" << std::endl;
  os << "\tRight Operand: [" << (Node)*rightOperand << "]" << std::endl;
}

TernaryOp::TernaryOp(Node* Left, Node* Middle, Node* Right) {
  this->depth = std::max(Left->depth, std::max(Middle->depth, Right->depth)) + 1;
  this->type = TERNARY_OP;
  this->leftOperand = Left;
  this->middleOperand = Middle;
  this->rightOperand = Right;
}

void TernaryOp::write(std::ostream &os) const {
  // Call parent class operator
  Node::write(os);

  // Print remaining data
  os << "\tLeft Operand: [" << (Node)*leftOperand << "]" << std::endl;
  os << "\tMiddle Operand: [" << (Node)*middleOperand << "]" << std::endl;
  os << "\tRight Operand: [" << (Node)*rightOperand << "]" << std::endl;
}
