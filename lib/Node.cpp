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

BinaryOp::BinaryOp(Node* Left, Node* Right, Op op) {
  this->leftOperand = Left;
  this->rightOperand = Right;
  this->op = op;
  this->type = BINARY_OP;
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
  this->leftOperand = Left;
  this->middleOperand = Middle;
  this->rightOperand = Right;
  this->type = TERNARY_OP;
}

void TernaryOp::write(std::ostream &os) const {
  // Call parent class operator
  Node::write(os);

  // Print remaining data
  os << "\tLeft Operand: [" << (Node)*leftOperand << "]" << std::endl;
  os << "\tMiddle Operand: [" << (Node)*middleOperand << "]" << std::endl;
  os << "\tRight Operand: [" << (Node)*rightOperand << "]" << std::endl;
}
