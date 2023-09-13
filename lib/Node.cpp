#include "../include/Node.h"

#include <iostream>

std::ostream &operator<<(std::ostream &os, const Node &node) {
  node.write(os);
  return os;
}

void Node::write(std::ostream &os) const {
  os << "\nID:" << id << std::endl;
  os << "\tDepth:" << depth << std::endl;
  os << "\tType:" << type << std::endl;
  os << "\tRounding:" << rounding << std::endl;
}

Integer::Integer(int value) {
  this->value = value;
  this->type = INTEGER;
}

void Integer::write(std::ostream &os) const {
  // Call parent class operator
  Node::write(os);

  // Print remaining data
  os << "\tValue:" << value << std::endl;
}

Float::Float(float value) {
  this->value = value;
  this->type = FLOAT;
}

void Float::write(std::ostream &os) const {
  // Call parent class operator
  Node::write(os);

  // Print remaining data
  os << "\tValue:" << value << std::endl;
}

Double::Double(double value) {
  this->value = value;
  this->type = DOUBLE;
}

void Double::write(std::ostream &os) const {
  // Call parent class operator
  Node::write(os);

  // Print remaining data
  os << "\tValue:" << value << std::endl;
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

VariableNode::VariableNode(string name) {
  this->name = name;
  this->type = VARIABLE;
}

void VariableNode::write(std::ostream &os) const {
  // Call parent class operator
  Node::write(os);

  // Print remaining data
  os << "\tValue:" << name << std::endl;
}

UnaryOp::UnaryOp(Node* Operand) {
  this->Operand = Operand;
  this->type = UNARY_OP;
}

void UnaryOp::write(std::ostream &os) const {
  // Call parent class operator
  Node::write(os);

  // Print remaining data
  os << "\tOperand: [" << *Operand << "]" << std::endl;
}

BinaryOp::BinaryOp(Node* Left, Node* Right) {
  this->leftOperand = Left;
  this->rightOperand = Right;
  this->type = BINARY_OP;
}

void BinaryOp::write(std::ostream &os) const {
  // Call parent class operator
  Node::write(os);

  // Print remaining data
  os << "\tLeft Operand: [" << *leftOperand << "]" << std::endl;
  os << "\tRight Operand: [" << *rightOperand << "]" << std::endl;
}

TernaryOp::TernaryOp(Node* Left, Node* Middle, Node* Right) {
  this->leftOperand = Left;
  this->middleOperand = Middle;
  this->rightOperand = Right;
  this->type = TERNARY_OP;
}

void TernaryOp::write(std::ostream &os) const {
  // Call parent class operator
  Node::write(os);

  // Print remaining data
  os << "\tLeft Operand: [" << *leftOperand << "]" << std::endl;
  os << "\tMiddle Operand: [" << *middleOperand << "]" << std::endl;
  os << "\tRight Operand: [" << *rightOperand << "]" << std::endl;
}
