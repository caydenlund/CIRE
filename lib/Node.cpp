#include "../include/Node.h"

#include <iostream>

int Node::NODE_COUNTER = 0;

void Node::write(std::ostream &os) const {
  os << "\nID:" << id << std::endl;
  os << "\tDepth:" << depth << std::endl;

  // Print type
  std::string type_string;
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
  os << "\tParents: [" << std::endl;
  for (auto parent : parents) {
    os << "\t" << parent->id << ", ";
  }
  os << "]" << std::endl;
}

ibex::ExprNode *Node::getExprNode() const {
  std::cout << "ERROR: Base class getExprNode called" << std::endl;
  return nullptr;
}

bool Node::operator==(const Node &other) const {
  return this->depth == other.depth &&
         this->type == other.type &&
         this->rounding == other.rounding;
}

Node *Node::operator+(Node &other) const {
  std::cout << "ERROR: Base class operator+ called" << std::endl;
  exit(1);
}

Node *Node::operator-(Node &other) const {
  std::cout << "ERROR: Base class operator+ called" << std::endl;
  exit(1);
}

Node *Node::operator*(Node &other) const {
  std::cout << "ERROR: Base class operator+ called" << std::endl;
  exit(1);
}

Node *Node::operator/(Node &other) const {
  std::cout << "ERROR: Base class operator+ called" << std::endl;
  exit(1);
}

ibex::ExprNode &Node::generateSymExpr() {
  std::cout << "ERROR: Base class generateSymExpr called. Base class does not "
               "have an Ibex Expression field"<< std::endl;
  exit(1);
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

bool Integer::operator==(const Integer &other) const {
  return Node::operator==(other) &&
         this->value == other.value;
}

Node *Integer::operator+(Node &other) const {
  ibex::ExprNode *a = this->getExprNode();
  ibex::ExprNode *b = other.getExprNode();

  const ibex::ExprConstant *c = (ibex::ExprConstant*)&ibex::ExprAdd::new_(*a, *b);

  return new Integer(*c);
}

Node *Integer::operator-(Node &other) const {
  ibex::ExprNode *a = this->getExprNode();
  ibex::ExprNode *b = other.getExprNode();

  const ibex::ExprConstant *c = (ibex::ExprConstant*)&ibex::ExprSub::new_(*a, *b);

  return new Integer(*c);
}

Node *Integer::operator*(Node &other) const {
  ibex::ExprNode *a = this->getExprNode();
  ibex::ExprNode *b = other.getExprNode();

  const ibex::ExprConstant *c = (ibex::ExprConstant*)&ibex::ExprMul::new_(*a, *b);

  return new Integer(*c);
}

Node *Integer::operator/(Node &other) const {
  ibex::ExprNode *a = this->getExprNode();
  ibex::ExprNode *b = other.getExprNode();

  const ibex::ExprConstant *c = (ibex::ExprConstant*)&ibex::ExprDiv::new_(*a, *b);

  return new Integer(*c);
}

ibex::ExprNode &Integer::generateSymExpr() {
  assert(this->value != nullptr && "ERROR: ibex::ExprConstant with Integer value should have been assigned while parsing/"
                                   "node creation\n");
  return *this->getExprNode();
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

bool Float::operator==(const Float &other) const {
  return Node::operator==(other) &&
         this->value == other.value;
}

Node *Float::operator+(Node &other) const {
  ibex::ExprNode *a = this->getExprNode();
  ibex::ExprNode *b = other.getExprNode();

  const ibex::ExprConstant *c = (ibex::ExprConstant*)&ibex::ExprAdd::new_(*a, *b);

  return new Float(*c);
}

Node *Float::operator-(Node &other) const {
  ibex::ExprNode *a = this->getExprNode();
  ibex::ExprNode *b = other.getExprNode();

  const ibex::ExprConstant *c = (ibex::ExprConstant*)&ibex::ExprSub::new_(*a, *b);

  return new Float(*c);
}

Node *Float::operator*(Node &other) const {
  ibex::ExprNode *a = this->getExprNode();
  ibex::ExprNode *b = other.getExprNode();

  const ibex::ExprConstant *c = (ibex::ExprConstant*)&ibex::ExprMul::new_(*a, *b);

  return new Float(*c);
}

Node *Float::operator/(Node &other) const {
  ibex::ExprNode *a = this->getExprNode();
  ibex::ExprNode *b = other.getExprNode();

  const ibex::ExprConstant *c = (ibex::ExprConstant*)&ibex::ExprDiv::new_(*a, *b);

  return new Float(*c);
}

ibex::ExprNode &Float::generateSymExpr() {
  assert(this->value != nullptr && "ERROR: ibex::ExprConstant with Float value should have been assigned while parsing/"
                                   "node creation\n");
  return *this->getExprNode();
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

bool Double::operator==(const Double &other) const {
  return Node::operator==(other) &&
         this->value == other.value;
}

Node *Double::operator+(Node &other) const {
  ibex::ExprNode *a = this->getExprNode();
  ibex::ExprNode *b = other.getExprNode();

  const ibex::ExprConstant *c = (ibex::ExprConstant*)&ibex::ExprAdd::new_(*a, *b);

  return new Double(*c);
}

Node *Double::operator-(Node &other) const {
  ibex::ExprNode *a = this->getExprNode();
  ibex::ExprNode *b = other.getExprNode();

  const ibex::ExprConstant *c = (ibex::ExprConstant*)&ibex::ExprSub::new_(*a, *b);

  return new Double(*c);
}

Node *Double::operator*(Node &other) const {
  ibex::ExprNode *a = this->getExprNode();
  ibex::ExprNode *b = other.getExprNode();

  const ibex::ExprConstant *c = (ibex::ExprConstant*)&ibex::ExprMul::new_(*a, *b);

  return new Double(*c);
}

Node *Double::operator/(Node &other) const {
  ibex::ExprNode *a = this->getExprNode();
  ibex::ExprNode *b = other.getExprNode();

  const ibex::ExprConstant *c = (ibex::ExprConstant*)&ibex::ExprDiv::new_(*a, *b);

  return new Double(*c);
}

ibex::ExprNode &Double::generateSymExpr() {
  assert(this->value != nullptr && "ERROR: ibex::ExprConstant with Double value should have been assigned while parsing/"
                                   "node creation\n");
  return *this->getExprNode();
}

FreeVariable::FreeVariable(const ibex::Interval &var) {
  this->var = &var;
  this->type = FREE_VARIABLE;
}

void FreeVariable::write(std::ostream &os) const {
  // Call parent class operator
  Node::write(os);

  // Print remaining data
  os << "\tValue:" << *var << std::endl;
}

Node *FreeVariable::operator+(Node &other) const {
  assert(other.type == FREE_VARIABLE);
  const ibex::Interval *a = this->var;
  const ibex::Interval *b = ((FreeVariable*)&other)->var;
  const ibex::Interval *c = new ibex::Interval(*a+*b);

  return new FreeVariable(*c);
}

Node *FreeVariable::operator-(Node &other) const {
  assert(other.type == FREE_VARIABLE);
  const ibex::Interval *a = this->var;
  const ibex::Interval *b = ((FreeVariable*)&other)->var;
  const ibex::Interval *c = new ibex::Interval(*a-*b);

  return new FreeVariable(*c);
}

Node *FreeVariable::operator*(Node &other) const {
  assert(other.type == FREE_VARIABLE);
  const ibex::Interval *a = this->var;
  const ibex::Interval *b = ((FreeVariable*)&other)->var;
  const ibex::Interval *c = new ibex::Interval(*a**b);

  return new FreeVariable(*c);
}

Node *FreeVariable::operator/(Node &other) const {
  assert(other.type == FREE_VARIABLE);
  const ibex::Interval *a = this->var;
  const ibex::Interval *b = ((FreeVariable*)&other)->var;
  const ibex::Interval *c = new ibex::Interval(*a/(*b));

  return new FreeVariable(*c);
}

ibex::ExprNode &FreeVariable::generateSymExpr() {
  assert(this->var != nullptr && "ERROR: ibex::Interval with Interval value should have been assigned while parsing/"
                                   "node creation\n");
  exit(1);
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

bool VariableNode::operator==(const VariableNode &other) const {
  return Node::operator==(other) &&
         this->variable == other.variable;
}

Node *VariableNode::operator+(Node &other) const {
  return new BinaryOp((Node *) this, (Node *) &other, BinaryOp::ADD);
}

Node *VariableNode::operator-(Node &other) const {
  return new BinaryOp((Node *) this, (Node *) &other, BinaryOp::SUB);
}

Node *VariableNode::operator*(Node &other) const {
  return new BinaryOp((Node *) this, (Node *) &other, BinaryOp::MUL);
}

Node *VariableNode::operator/(Node &other) const {
  return new BinaryOp((Node *) this, (Node *) &other, BinaryOp::DIV);
}

ibex::ExprNode &VariableNode::generateSymExpr() {
  assert(this->variable != nullptr && "ERROR: ibex::ExprSymbol with string literal should have been assigned while parsing/"
                                   "node creation\n");
  return *this->getExprNode();
}

UnaryOp::UnaryOp(Node* Operand, Op op, const ibex::ExprUnaryOp &expr) {
  this->depth = Operand->depth + 1;
  this->type = UNARY_OP;
  this->Operand = Operand;
  this->op = op;
  this->expr = &expr;
  this->Operand->parents.insert(this);
}

void UnaryOp::write(std::ostream &os) const {
  // Call parent class operator
  Node::write(os);

  // Print remaining data
  os << "\tOperand: [" << (Node)*Operand << "]" << std::endl;
}

bool UnaryOp::operator==(const UnaryOp &other) const {
  return Node::operator==(other) &&
          this->Operand == other.Operand &&
          this->op == other.op;
}

Node *UnaryOp::operator+(Node &other) const {
  return new BinaryOp((Node *) this, (Node *) &other, BinaryOp::ADD);
}

Node *UnaryOp::operator-(Node &other) const {
  return new BinaryOp((Node *) this, (Node *) &other, BinaryOp::SUB);
}

Node *UnaryOp::operator*(Node &other) const {
  return new BinaryOp((Node *) this, (Node *) &other, BinaryOp::MUL);
}

Node *UnaryOp::operator/(Node &other) const {
  return new BinaryOp((Node *) this, (Node *) &other, BinaryOp::DIV);
}

ibex::ExprNode *UnaryOp::getExprNode() const {
  return (ibex::ExprNode*)this->expr;
}

ibex::ExprNode &UnaryOp::generateSymExpr() {
  // TODO: Generate ibex expressions after adding an enum Op with Unary operations
  return Node::generateSymExpr();
}

BinaryOp::BinaryOp(Node* Left, Node* Right, Op op) {
  this->depth = std::max(Left->depth, Right->depth) + 1;
  this->type = BINARY_OP;
  this->leftOperand = Left;
  this->rightOperand = Right;
  this->op = op;
  this->expr = nullptr;
  this->leftOperand->parents.insert(this);
  this->rightOperand->parents.insert(this);
}

void BinaryOp::write(std::ostream &os) const {
  // Call parent class operator
  Node::write(os);

  // Print remaining data
  os << "\tLeft Operand: [" << (Node)*leftOperand << "]" << std::endl;
  // Print operator
  std::string operator_string;
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

bool BinaryOp::operator==(const BinaryOp &other) const {
  return Node::operator==(other) &&
          this->leftOperand == other.leftOperand &&
          this->rightOperand == other.rightOperand &&
          this->op == other.op;
}

ibex::ExprNode *BinaryOp::getExprNode() const {
  return (ibex::ExprNode*)this->expr;
}

ibex::ExprNode &BinaryOp::generateSymExpr() {
  switch (op) {
    case ADD: return (ibex::ExprNode &) ibex::ExprAdd::new_(*leftOperand->getExprNode(), *rightOperand->getExprNode());
    case SUB: return (ibex::ExprNode &) ibex::ExprSub::new_(*leftOperand->getExprNode(), *rightOperand->getExprNode());
    case MUL: return (ibex::ExprNode &) ibex::ExprMul::new_(*leftOperand->getExprNode(), *rightOperand->getExprNode());
    case DIV: return (ibex::ExprNode &) ibex::ExprDiv::new_(*leftOperand->getExprNode(), *rightOperand->getExprNode());
    default: return (ibex::ExprNode &) ibex::ExprAdd::new_(*leftOperand->getExprNode(), *rightOperand->getExprNode());
  }
}

Node *BinaryOp::operator+(Node &other) const {
  return new BinaryOp((Node *) this, (Node *) &other, BinaryOp::ADD);
}

Node *BinaryOp::operator-(Node &other) const {
  return new BinaryOp((Node *) this, (Node *) &other, BinaryOp::SUB);
}

Node *BinaryOp::operator*(Node &other) const {
  return new BinaryOp((Node *) this, (Node *) &other, BinaryOp::MUL);
}

Node *BinaryOp::operator/(Node &other) const {
  return new BinaryOp((Node *) this, (Node *) &other, BinaryOp::DIV);
}

TernaryOp::TernaryOp(Node* Left, Node* Middle, Node* Right) {
  this->depth = std::max(Left->depth, std::max(Middle->depth, Right->depth)) + 1;
  this->type = TERNARY_OP;
  this->leftOperand = Left;
  this->middleOperand = Middle;
  this->rightOperand = Right;
  this->leftOperand->parents.insert(this);
  this->middleOperand->parents.insert(this);
  this->rightOperand->parents.insert(this);
}

void TernaryOp::write(std::ostream &os) const {
  // Call parent class operator
  Node::write(os);

  // Print remaining data
  os << "\tLeft Operand: [" << (Node)*leftOperand << "]" << std::endl;
  os << "\tMiddle Operand: [" << (Node)*middleOperand << "]" << std::endl;
  os << "\tRight Operand: [" << (Node)*rightOperand << "]" << std::endl;
}

bool TernaryOp::operator==(const TernaryOp &other) const {
  return Node::operator==(other) &&
          this->leftOperand == other.leftOperand &&
          this->middleOperand == other.middleOperand &&
          this->rightOperand == other.rightOperand;
}

Node *TernaryOp::operator+(Node &other) const {
  return new BinaryOp((Node *) this, (Node *) &other, BinaryOp::ADD);
}

Node *TernaryOp::operator-(Node &other) const {
  return new BinaryOp((Node *) this, (Node *) &other, BinaryOp::SUB);
}

Node *TernaryOp::operator*(Node &other) const {
  return new BinaryOp((Node *) this, (Node *) &other, BinaryOp::MUL);
}

Node *TernaryOp::operator/(Node &other) const {
  return new BinaryOp((Node *) this, (Node *) &other, BinaryOp::DIV);
}

ibex::ExprNode *TernaryOp::getExprNode() const {
  return Node::getExprNode();
}

ibex::ExprNode &TernaryOp::generateSymExpr() {
  // TODO: Generate ibex expressions after adding an enum Op with Unary operations
  return Node::generateSymExpr();
}

std::ostream &operator<<(std::ostream &os, const Node &node) {
  node.write(os);
  return os;
}

Node *operator+(Node &x, Node *y) {
  return x+*y;
}

Node *operator-(Node &x, Node *y) {
  return x-*y;
}

Node *operator*(Node &x, Node *y) {
  return x**y;
}

Node *operator/(Node &x, Node *y) {
  return x/(*y);
}
