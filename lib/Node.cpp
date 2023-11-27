#include "../include/Node.h"

#include <iostream>

int Node::NODE_COUNTER = 0;

bool Node::isInteger() const {
  return type == INTEGER;
}

bool Node::isFloat() const {
  return type == FLOAT;
}

bool Node::isDouble() const {
  return type == DOUBLE;
}

bool Node::isFreeVariable() const {
  return type == FREE_VARIABLE;
}

bool Node::isVariable() const {
  return type == VARIABLE;
}

bool Node::isUnaryOp() const {
  return type == UNARY_OP;
}

bool Node::isBinaryOp() const {
  return type == BINARY_OP;
}

bool Node::isTernaryOp() const {
  return type == TERNARY_OP;
}

void Node::setRounding(Node::RoundingType roundingType) {
  rounding = RoundingAmount[roundingType];
}

void Node::setAbsoluteError(const ibex::ExprNode *absErr) {
  absoluteError = absErr;
}

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
  return depth == other.depth &&
         type == other.type &&
         rounding == other.rounding;
}

Node &Node::operator+(Node &other) const {
  std::cout << "ERROR: Base class operator+ called" << std::endl;
  exit(1);
}

Node &Node::operator-(Node &other) const {
  std::cout << "ERROR: Base class operator+ called" << std::endl;
  exit(1);
}

Node &Node::operator*(Node &other) const {
  std::cout << "ERROR: Base class operator+ called" << std::endl;
  exit(1);
}

Node &Node::operator/(Node &other) const {
  std::cout << "ERROR: Base class operator+ called" << std::endl;
  exit(1);
}

double Node::getRounding() {
  return rounding;
}

ibex::ExprNode &Node::getAbsoluteError() {
  return (ibex::ExprNode &) *getExprNode();
}

ibex::ExprNode &Node::generateSymExpr() {
  std::cout << "ERROR: Base class generateSymExpr called. Base class does not "
               "have an Ibex Expression field"<< std::endl;
  exit(1);
}

Node *Node::getChildNode(int index) const {
  std::cout << "ERROR: Base class getChildNode called. Base class does not "
               "have child nodes"<< std::endl;
  exit(1);
}

Integer::Integer(const ibex::ExprConstant &value): value(&value) {
  type = INTEGER;
}

void Integer::write(std::ostream &os) const {
  // Call parent class operator
  Node::write(os);

  // Print remaining data
  os << "\tValue:" << value << std::endl;
}

ibex::ExprNode *Integer::getExprNode() const {
  return (ibex::ExprNode*)value;
}

bool Integer::operator==(const Integer &other) const {
  return Node::operator==(other) &&
         value == other.value;
}

Node &Integer::operator+(Node &other) const {
  ibex::ExprNode *a = getExprNode();

  if (other.isInteger()) {
    ibex::ExprNode *b = other.getExprNode();
    const ibex::ExprConstant *c = (ibex::ExprConstant*)&ibex::ExprAdd::new_(*a, *b);
    return *new Integer(*c);
  } else if(other.isFloat()) {
    ibex::ExprNode *b = other.getExprNode();
    const ibex::ExprConstant *c = (ibex::ExprConstant*)&ibex::ExprAdd::new_(*a, *b);
    return *new Float(*c);
  } else if(other.isDouble()) {
    ibex::ExprNode *b = other.getExprNode();
    const ibex::ExprConstant *c = (ibex::ExprConstant*)&ibex::ExprAdd::new_(*a, *b);
    return *new Double(*c);
  }

  return *new BinaryOp((Node *) this, (Node *) &other, BinaryOp::ADD);
}

Node &Integer::operator-(Node &other) const {
  ibex::ExprNode *a = getExprNode();

  if (other.isInteger()) {
    ibex::ExprNode *b = other.getExprNode();
    const ibex::ExprConstant *c = (ibex::ExprConstant*)&ibex::ExprSub::new_(*a, *b);
    return *new Integer(*c);
  } else if(other.isFloat()) {
    ibex::ExprNode *b = other.getExprNode();
    const ibex::ExprConstant *c = (ibex::ExprConstant*)&ibex::ExprSub::new_(*a, *b);
    return *new Float(*c);
  } else if(other.isDouble()) {
    ibex::ExprNode *b = other.getExprNode();
    const ibex::ExprConstant *c = (ibex::ExprConstant*)&ibex::ExprSub::new_(*a, *b);
    return *new Double(*c);
  }

  return *new BinaryOp((Node *) this, (Node *) &other, BinaryOp::SUB);
}

Node &Integer::operator*(Node &other) const {
  ibex::ExprNode *a = getExprNode();

  if (other.isInteger()) {
    ibex::ExprNode *b = other.getExprNode();
    const ibex::ExprConstant *c = (ibex::ExprConstant*)&ibex::ExprMul::new_(*a, *b);
    return *new Integer(*c);
  } else if(other.isFloat()) {
    ibex::ExprNode *b = other.getExprNode();
    const ibex::ExprConstant *c = (ibex::ExprConstant*)&ibex::ExprMul::new_(*a, *b);
    return *new Float(*c);
  } else if(other.isDouble()) {
    ibex::ExprNode *b = other.getExprNode();
    const ibex::ExprConstant *c = (ibex::ExprConstant*)&ibex::ExprMul::new_(*a, *b);
    return *new Double(*c);
  }

  return *new BinaryOp((Node *) this, (Node *) &other, BinaryOp::MUL);
}

Node &Integer::operator/(Node &other) const {
  ibex::ExprNode *a = getExprNode();

  if (other.isInteger()) {
    ibex::ExprNode *b = other.getExprNode();
    const ibex::ExprConstant *c = (ibex::ExprConstant*)&ibex::ExprDiv::new_(*a, *b);
    return *new Integer(*c);
  } else if(other.isFloat()) {
    ibex::ExprNode *b = other.getExprNode();
    const ibex::ExprConstant *c = (ibex::ExprConstant*)&ibex::ExprDiv::new_(*a, *b);
    return *new Float(*c);
  } else if(other.isDouble()) {
    ibex::ExprNode *b = other.getExprNode();
    const ibex::ExprConstant *c = (ibex::ExprConstant*)&ibex::ExprDiv::new_(*a, *b);
    return *new Double(*c);
  }

  return *new BinaryOp((Node *) this, (Node *) &other, BinaryOp::DIV);
}

ibex::ExprNode &Integer::generateSymExpr() {
  assert(value != nullptr && "ERROR: ibex::ExprConstant with Integer value should have been assigned while parsing/"
                                   "node creation\n");
  return *getExprNode();
}

Node *Integer::getChildNode(int index) const {
  std::cout << "ERROR: Integer Class does not have child nodes" << std::endl;
  exit(1);
}


Float::Float(const ibex::ExprConstant &value):value(&value) {
  type = FLOAT;
}

void Float::write(std::ostream &os) const {
  // Call parent class operator
  Node::write(os);

  // Print remaining data
  os << "\tValue:" << value << std::endl;
}

ibex::ExprNode *Float::getExprNode() const {
  return (ibex::ExprNode*)value;
}

bool Float::operator==(const Float &other) const {
  return Node::operator==(other) &&
         value == other.value;
}

Node &Float::operator+(Node &other) const {
  ibex::ExprNode *a = getExprNode();

  if(other.isFloat() || other.isInteger()) {
    ibex::ExprNode *b = other.getExprNode();
    const ibex::ExprConstant *c = (ibex::ExprConstant*)&ibex::ExprAdd::new_(*a, *b);
    return *new Float(*c);
  } else if(other.isDouble()) {
    ibex::ExprNode *b = other.getExprNode();
    const ibex::ExprConstant *c = (ibex::ExprConstant*)&ibex::ExprAdd::new_(*a, *b);
    return *new Double(*c);
  }

  return *new BinaryOp((Node *) this, (Node *) &other, BinaryOp::ADD);
}

Node &Float::operator-(Node &other) const {
  ibex::ExprNode *a = getExprNode();

  if(other.isFloat() || other.isInteger()) {
    ibex::ExprNode *b = other.getExprNode();
    const ibex::ExprConstant *c = (ibex::ExprConstant*)&ibex::ExprSub::new_(*a, *b);
    return *new Float(*c);
  } else if(other.isDouble()) {
    ibex::ExprNode *b = other.getExprNode();
    const ibex::ExprConstant *c = (ibex::ExprConstant*)&ibex::ExprSub::new_(*a, *b);
    return *new Double(*c);
  }

  return *new BinaryOp((Node *) this, (Node *) &other, BinaryOp::SUB);
}

Node &Float::operator*(Node &other) const {
  ibex::ExprNode *a = getExprNode();

  if(other.isFloat() || other.isInteger()) {
    ibex::ExprNode *b = other.getExprNode();
    const ibex::ExprConstant *c = (ibex::ExprConstant*)&ibex::ExprMul::new_(*a, *b);
    return *new Float(*c);
  } else if(other.isDouble()) {
    ibex::ExprNode *b = other.getExprNode();
    const ibex::ExprConstant *c = (ibex::ExprConstant*)&ibex::ExprMul::new_(*a, *b);
    return *new Double(*c);
  }

  return *new BinaryOp((Node *) this, (Node *) &other, BinaryOp::MUL);
}

Node &Float::operator/(Node &other) const {
  ibex::ExprNode *a = getExprNode();

  if(other.isFloat() || other.isInteger()) {
    ibex::ExprNode *b = other.getExprNode();
    const ibex::ExprConstant *c = (ibex::ExprConstant*)&ibex::ExprDiv::new_(*a, *b);
    return *new Float(*c);
  } else if(other.isDouble()) {
    ibex::ExprNode *b = other.getExprNode();
    const ibex::ExprConstant *c = (ibex::ExprConstant*)&ibex::ExprDiv::new_(*a, *b);
    return *new Double(*c);
  }

  return *new BinaryOp((Node *) this, (Node *) &other, BinaryOp::DIV);
}

ibex::ExprNode &Float::getAbsoluteError() {
  return (ibex::ExprNode &) *absoluteError;
}

ibex::ExprNode &Float::generateSymExpr() {
  assert(value != nullptr && "ERROR: ibex::ExprConstant with Float value should have been assigned while parsing/"
                                   "node creation\n");
  return *getExprNode();
}

Node *Float::getChildNode(int index) const {
  std::cout << "ERROR: Float Class does not have child nodes" << std::endl;
  exit(1);
}

Double::Double(const ibex::ExprConstant &value): value(&value) {
  type = DOUBLE;
}

void Double::write(std::ostream &os) const {
  // Call parent class operator
  Node::write(os);

  // Print remaining data
  os << "\tValue:" << value << std::endl;
}

ibex::ExprNode *Double::getExprNode() const {
  return (ibex::ExprNode*)value;
}

bool Double::operator==(const Double &other) const {
  return Node::operator==(other) &&
         value == other.value;
}

Node &Double::operator+(Node &other) const {
  ibex::ExprNode *a = getExprNode();

  if(other.isDouble() || other.isFloat() || other.isInteger()) {
    ibex::ExprNode *b = other.getExprNode();
    const ibex::ExprConstant *c = (ibex::ExprConstant*)&ibex::ExprAdd::new_(*a, *b);
    return *new Double(*c);
  }

  return *new BinaryOp((Node *) this, (Node *) &other, BinaryOp::ADD);
}

Node &Double::operator-(Node &other) const {
  ibex::ExprNode *a = getExprNode();

  if(other.isDouble() || other.isFloat() || other.isInteger()) {
    ibex::ExprNode *b = other.getExprNode();
    const ibex::ExprConstant *c = (ibex::ExprConstant*)&ibex::ExprSub::new_(*a, *b);
    return *new Double(*c);
  }

  return *new BinaryOp((Node *) this, (Node *) &other, BinaryOp::SUB);
}

Node &Double::operator*(Node &other) const {
  ibex::ExprNode *a = getExprNode();

  if(other.isDouble() || other.isFloat() || other.isInteger()) {
    ibex::ExprNode *b = other.getExprNode();
    const ibex::ExprConstant *c = (ibex::ExprConstant*)&ibex::ExprMul::new_(*a, *b);
    return *new Double(*c);
  }

  return *new BinaryOp((Node *) this, (Node *) &other, BinaryOp::MUL);
}

Node &Double::operator/(Node &other) const {
  ibex::ExprNode *a = getExprNode();

  if(other.isDouble() || other.isFloat() || other.isInteger()) {
    ibex::ExprNode *b = other.getExprNode();
    const ibex::ExprConstant *c = (ibex::ExprConstant*)&ibex::ExprDiv::new_(*a, *b);
    return *new Double(*c);
  }

  return *new BinaryOp((Node *) this, (Node *) &other, BinaryOp::DIV);
}

ibex::ExprNode &Double::getAbsoluteError() {
  return (ibex::ExprNode &) *absoluteError;
}

ibex::ExprNode &Double::generateSymExpr() {
  assert(value != nullptr && "ERROR: ibex::ExprConstant with Double value should have been assigned while parsing/"
                                   "node creation\n");
  return *getExprNode();
}

Node *Double::getChildNode(int index) const {
  std::cout << "ERROR: Double Class does not have child nodes" << std::endl;
  exit(1);
}

FreeVariable::FreeVariable(const ibex::Interval &var): var(&var) {
  type = FREE_VARIABLE;
}

void FreeVariable::write(std::ostream &os) const {
  // Call parent class operator
  Node::write(os);

  // Print remaining data
  os << "\tValue:" << *var << std::endl;
}

Node &FreeVariable::operator+(Node &other) const {
  assert(other.type == FREE_VARIABLE);
  const ibex::Interval *a = var;
  const ibex::Interval *b = ((FreeVariable*)&other)->var;
  const ibex::Interval *c = new ibex::Interval(*a+*b);

  return *new FreeVariable(*c);
}

Node &FreeVariable::operator-(Node &other) const {
  assert(other.type == FREE_VARIABLE);
  const ibex::Interval *a = var;
  const ibex::Interval *b = ((FreeVariable*)&other)->var;
  const ibex::Interval *c = new ibex::Interval(*a-*b);

  return *new FreeVariable(*c);
}

Node &FreeVariable::operator*(Node &other) const {
  assert(other.type == FREE_VARIABLE);
  const ibex::Interval *a = var;
  const ibex::Interval *b = ((FreeVariable*)&other)->var;
  const ibex::Interval *c = new ibex::Interval(*a**b);

  return *new FreeVariable(*c);
}

Node &FreeVariable::operator/(Node &other) const {
  assert(other.type == FREE_VARIABLE);
  const ibex::Interval *a = var;
  const ibex::Interval *b = ((FreeVariable*)&other)->var;
  const ibex::Interval *c = new ibex::Interval(*a/(*b));

  return *new FreeVariable(*c);
}

ibex::ExprNode &FreeVariable::getAbsoluteError() {
  return (ibex::ExprNode &) *absoluteError;
}

ibex::ExprNode &FreeVariable::generateSymExpr() {
  assert(var != nullptr && "ERROR: ibex::Interval with Interval value should have been assigned while parsing/"
                                   "node creation\n");
  exit(1);
}

Node *FreeVariable::getChildNode(int index) const {
  std::cout << "ERROR: FreeVariable Class does not have child nodes" << std::endl;
  exit(1);
}

VariableNode::VariableNode(const ibex::ExprSymbol& variable): variable(&variable) {
  type = VARIABLE;
}

void VariableNode::write(std::ostream &os) const {
  // Call parent class operator
  Node::write(os);

  // Print remaining data
  os << "\tVariable:" << *variable << std::endl;
}

ibex::ExprNode *VariableNode::getExprNode() const {
  return (ibex::ExprNode*)variable;
}

bool VariableNode::operator==(const VariableNode &other) const {
  return Node::operator==(other) &&
         variable == other.variable;
}

Node &VariableNode::operator+(Node &other) const {
  return *new BinaryOp((Node *) this, (Node *) &other, BinaryOp::ADD);
}

Node &VariableNode::operator-(Node &other) const {
  return *new BinaryOp((Node *) this, (Node *) &other, BinaryOp::SUB);
}

Node &VariableNode::operator*(Node &other) const {
  return *new BinaryOp((Node *) this, (Node *) &other, BinaryOp::MUL);
}

Node &VariableNode::operator/(Node &other) const {
  return *new BinaryOp((Node *) this, (Node *) &other, BinaryOp::DIV);
}

ibex::ExprNode &VariableNode::getAbsoluteError() {
  return (ibex::ExprNode &) *absoluteError;
}

ibex::ExprNode &VariableNode::generateSymExpr() {
  assert(variable != nullptr && "ERROR: ibex::ExprSymbol with string literal should have been assigned while parsing/"
                                   "node creation\n");
  return *getExprNode();
}

Node *VariableNode::getChildNode(int index) const {
  std::cout << "ERROR: VariableNode Class does not have child nodes" << std::endl;
  exit(1);
}

UnaryOp::UnaryOp(Node* Operand, Op op): Operand(Operand),
                                        op(op),
                                        expr(nullptr) {
  depth = Operand->depth + 1;
  type = UNARY_OP;
  Operand->parents.insert(this);
}

void UnaryOp::write(std::ostream &os) const {
  // Call parent class operator
  Node::write(os);

  // Print remaining data
  os << "\tOperand: [" << (Node)*Operand << "]" << std::endl;
}

ibex::ExprNode *UnaryOp::getExprNode() const {
  return (ibex::ExprNode*)expr;
}

bool UnaryOp::operator==(const UnaryOp &other) const {
  return Node::operator==(other) &&
          Operand == other.Operand &&
          op == other.op;
}

Node &UnaryOp::operator+(Node &other) const {
  return *new BinaryOp((Node *) this, (Node *) &other, BinaryOp::ADD);
}

Node &UnaryOp::operator-(Node &other) const {
  return *new BinaryOp((Node *) this, (Node *) &other, BinaryOp::SUB);
}

Node &UnaryOp::operator*(Node &other) const {
  return *new BinaryOp((Node *) this, (Node *) &other, BinaryOp::MUL);
}

Node &UnaryOp::operator/(Node &other) const {
  return *new BinaryOp((Node *) this, (Node *) &other, BinaryOp::DIV);
}

double UnaryOp::getRounding() {
  return rounding * OpErrorULPs[op];
}

ibex::ExprNode &UnaryOp::generateSymExpr() {
  switch (op) {
    case NEG: return (ibex::ExprNode &) -(*Operand->getExprNode());
    case SIN: return (ibex::ExprNode &) ibex::sin(*Operand->getExprNode());
    case COS: return (ibex::ExprNode &) ibex::cos(*Operand->getExprNode());
    case TAN: return (ibex::ExprNode &) ibex::tan(*Operand->getExprNode());
    case SINH: return (ibex::ExprNode &) ibex::sinh(*Operand->getExprNode());
    case COSH: return (ibex::ExprNode &) ibex::cosh(*Operand->getExprNode());
    case TANH: return (ibex::ExprNode &) ibex::tanh(*Operand->getExprNode());
    case ASIN: return (ibex::ExprNode &) ibex::asin(*Operand->getExprNode());
    case ACOS: return (ibex::ExprNode &) ibex::acos(*Operand->getExprNode());
    case ATAN: return (ibex::ExprNode &) ibex::atan(*Operand->getExprNode());
    case LOG: return (ibex::ExprNode &) ibex::log(*Operand->getExprNode());
    case SQRT: return (ibex::ExprNode &) ibex::sqrt(*Operand->getExprNode());
    case EXP: return (ibex::ExprNode &) ibex::exp(*Operand->getExprNode());
    default:
      std::cout << "ERROR: Unknown operator" << std::endl;
      exit(1);
  }
}

Node *UnaryOp::getChildNode(int index) const {
  if (index == 0) {
    return Operand;
  } else {
    std::cout << "ERROR: UnaryOp Class has only one child node" << std::endl;
    exit(1);
  }
}

BinaryOp::BinaryOp(Node* Left, Node* Right, Op op): leftOperand(Left),
                                                    rightOperand(Right),
                                                    op(op),
                                                    expr(nullptr) {
  depth = std::max(Left->depth, Right->depth) + 1;
  type = BINARY_OP;
  leftOperand->parents.insert(this);
  rightOperand->parents.insert(this);
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
          leftOperand == other.leftOperand &&
          rightOperand == other.rightOperand &&
          op == other.op;
}

ibex::ExprNode *BinaryOp::getExprNode() const {
  return (ibex::ExprNode*)expr;
}

Node &BinaryOp::operator+(Node &other) const {
  return *new BinaryOp((Node *) this, (Node *) &other, BinaryOp::ADD);
}

Node &BinaryOp::operator-(Node &other) const {
  return *new BinaryOp((Node *) this, (Node *) &other, BinaryOp::SUB);
}

Node &BinaryOp::operator*(Node &other) const {
  return *new BinaryOp((Node *) this, (Node *) &other, BinaryOp::MUL);
}

Node &BinaryOp::operator/(Node &other) const {
  return *new BinaryOp((Node *) this, (Node *) &other, BinaryOp::DIV);
}

double BinaryOp::getRounding() {
  return rounding * OpErrorULPs[op];
}

ibex::ExprNode &BinaryOp::generateSymExpr() {
  switch (op) {
    case ADD: return (ibex::ExprNode &) (*leftOperand->getExprNode() + *rightOperand->getExprNode());
    case SUB: return (ibex::ExprNode &) (*leftOperand->getExprNode() - *rightOperand->getExprNode());
    case MUL: return (ibex::ExprNode &) (*leftOperand->getExprNode() * *rightOperand->getExprNode());
    case DIV: return (ibex::ExprNode &) (*leftOperand->getExprNode() / *rightOperand->getExprNode());
    default: return (ibex::ExprNode &) (*leftOperand->getExprNode() + *rightOperand->getExprNode());
  }
}

Node *BinaryOp::getChildNode(int index) const {
  if (index == 0) {
    return leftOperand;
  } else if (index == 1) {
    return rightOperand;
  } else {
    std::cout << "ERROR: BinaryOp Class has only two child nodes" << std::endl;
    exit(1);
  }
}

TernaryOp::TernaryOp(Node* Left, Node* Middle, Node* Right): leftOperand(Left),
                                                              middleOperand(Middle),
                                                              rightOperand(Right) {
  depth = std::max(Left->depth, std::max(Middle->depth, Right->depth)) + 1;
  type = TERNARY_OP;
  leftOperand->parents.insert(this);
  middleOperand->parents.insert(this);
  rightOperand->parents.insert(this);
}

void TernaryOp::write(std::ostream &os) const {
  // Call parent class operator
  Node::write(os);

  // Print remaining data
  os << "\tLeft Operand: [" << (Node)*leftOperand << "]" << std::endl;
  os << "\tMiddle Operand: [" << (Node)*middleOperand << "]" << std::endl;
  os << "\tRight Operand: [" << (Node)*rightOperand << "]" << std::endl;
}

ibex::ExprNode *TernaryOp::getExprNode() const {
  return Node::getExprNode();
}

bool TernaryOp::operator==(const TernaryOp &other) const {
  return Node::operator==(other) &&
          leftOperand == other.leftOperand &&
          middleOperand == other.middleOperand &&
          rightOperand == other.rightOperand;
}

Node &TernaryOp::operator+(Node &other) const {
  return *new BinaryOp((Node *) this, (Node *) &other, BinaryOp::ADD);
}

Node &TernaryOp::operator-(Node &other) const {
  return *new BinaryOp((Node *) this, (Node *) &other, BinaryOp::SUB);
}

Node &TernaryOp::operator*(Node &other) const {
  return *new BinaryOp((Node *) this, (Node *) &other, BinaryOp::MUL);
}

Node &TernaryOp::operator/(Node &other) const {
  return *new BinaryOp((Node *) this, (Node *) &other, BinaryOp::DIV);
}

double TernaryOp::getRounding() {
  // TODO: Add rounding for Ternary operations
  return rounding;
}

ibex::ExprNode &TernaryOp::generateSymExpr() {
  // TODO: Generate ibex expressions after adding an enum Op with Unary operations
  return Node::generateSymExpr();
}

Node *TernaryOp::getChildNode(int index) const {
  if (index == 0) {
    return leftOperand;
  } else if (index == 1) {
    return middleOperand;
  } else if (index == 2) {
    return rightOperand;
  } else {
    std::cout << "ERROR: TernaryOp Class has only three child nodes" << std::endl;
    exit(1);
  }
}

std::ostream &operator<<(std::ostream &os, const Node &node) {
  node.write(os);
  return os;
}

Node &operator+(Node &x, Node *y) {
  return x+*y;
}

Node &operator-(Node &x, Node *y) {
  return x-*y;
}

Node &operator*(Node &x, Node *y) {
  return x**y;
}

Node &operator/(Node &x, Node *y) {
  return x/(*y);
}

Node &operator-(Node &x) {
  return *new UnaryOp(&x, UnaryOp::NEG);
}

const ibex::ExprNode& product(const ibex::ExprNode& left, const ibex::ExprNode& right) {
  if (left == ibex::ExprConstant::new_scalar(1.0) && right == ibex::ExprConstant::new_scalar(1.0)) {
    return ibex::ExprConstant::new_scalar(1.0);
  } else if (left == ibex::ExprConstant::new_scalar(1.0)) {
    return right;
  } else if (left == ibex::ExprConstant::new_scalar(-1.0)) {
    return -right;
  } else if (right == ibex::ExprConstant::new_scalar(1.0)) {
    return left;
  } else if (right == ibex::ExprConstant::new_scalar(-1.0)) {
    return -left;
  } else {
    return ibex::ExprMul::new_(left, right);
  }
}

const ibex::ExprNode& product(const ibex::ExprNode& left, double right) {
if (left == ibex::ExprConstant::new_scalar(1.0) && right == 1.0) {
    return ibex::ExprConstant::new_scalar(1.0);
  } else if (left == ibex::ExprConstant::new_scalar(1.0)) {
    return ibex::ExprConstant::new_scalar(right);
  } else if (left == ibex::ExprConstant::new_scalar(-1.0)) {
    return ibex::ExprConstant::new_scalar(-right);
  } else if (right == 1.0) {
    return left;
  } else if (right == -1.0) {
    return -left;
  } else {
    return left*ibex::ExprConstant::new_scalar(right);
  }
}