#include "../include/Node.h"

#include <iostream>

int Node::NEW_FREE_VARIABLE_COUNTER = 0;
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

void Node::setRoundingType(Node::RoundingType OpRndType) {
  OpRndType = OpRndType;
}

void Node::setRoundingFromType(RoundingType OpRndType) {
  OpRndType = OpRndType;
  OpRounding = RoundingAmount[OpRndType];
}

void Node::setRounding(double OpRounding) {
  this->OpRounding = OpRounding;
}

void Node::setAbsoluteError(const ibex::ExprNode *absErr) {
  absoluteError = absErr;
}

Node::RoundingType Node::getRoundingType() {
  return OpRndType;
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
  os << "\tRounding:" << OpRounding << std::endl;
  os << "\tParents: [" << std::endl;
  for (auto parent : parents) {
    os << "\t" << parent->id << ", ";
  }
  os << "]" << std::endl;
}

ibex::ExprNode *Node::getExprNode() const {
  std::cout << "ERROR: Base class getExprNode called" << std::endl;
  exit(1);
}

bool Node::operator==(const Node &other) const {
  return depth == other.depth &&
         type == other.type &&
         OpRounding == other.OpRounding;
}

Node &Node::operator+(Node &other) const {
  std::cout << "ERROR: Base class operator+ called" << std::endl;
  exit(1);
}

Node &Node::operator-(Node &other) const {
  std::cout << "ERROR: Base class operator- called" << std::endl;
  exit(1);
}

Node &Node::operator*(Node &other) const {
  std::cout << "ERROR: Base class operator* called" << std::endl;
  exit(1);
}

Node &Node::operator/(Node &other) const {
  std::cout << "ERROR: Base class operator/ called" << std::endl;
  exit(1);
}

double Node::getRounding() {
  return OpRounding;
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

Integer::Integer(const int val): val(val) {
  type = INTEGER;
  OpRndType = INT;
  OpRounding = RoundingAmount[INT];
}

Integer::Integer(const ibex::ExprConstant &value): value(&value) {
  type = INTEGER;
  OpRndType = INT;
  OpRounding = RoundingAmount[INT];
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
  if (other.isInteger()) {
    return *new Integer(val + ((Integer*)&other)->val);
  } else if(other.isFloat()) {
    return *new Float(val + ((Float*)&other)->val);
  } else if(other.isDouble()) {
    return *new Double(val + ((Double*)&other)->val);
  }

  return *new BinaryOp((Node *) this, (Node *) &other, ADD);
}

Node &Integer::operator-(Node &other) const {
  if (other.isInteger()) {
    return *new Integer(val - ((Integer*)&other)->val);
  } else if(other.isFloat()) {
    return *new Float(val - ((Float*)&other)->val);
  } else if(other.isDouble()) {
    return *new Double(val - ((Double*)&other)->val);
  }

  return *new BinaryOp((Node *) this, (Node *) &other, SUB);
}

Node &Integer::operator*(Node &other) const {
  if (other.isInteger()) {
    return *new Integer(val * ((Integer*)&other)->val);
  } else if(other.isFloat()) {
    return *new Float(val * ((Float*)&other)->val);
  } else if(other.isDouble()) {
    return *new Double(val * ((Double*)&other)->val);
  }

  return *new BinaryOp((Node *) this, (Node *) &other, MUL);
}

Node &Integer::operator/(Node &other) const {
  if (other.isInteger()) {
    return *new Integer(val / ((Integer*)&other)->val);
  } else if(other.isFloat()) {
    return *new Float(val / ((Float*)&other)->val);
  } else if(other.isDouble()) {
    return *new Double(val / ((Double*)&other)->val);
  }

  return *new BinaryOp((Node *) this, (Node *) &other, DIV);
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

Float::Float(const float val): val(val) {
  type = FLOAT;
  OpRndType = FL32;
  OpRounding = RoundingAmount[FL32];
}

Float::Float(const ibex::ExprConstant &value):value(&value) {
  type = FLOAT;
  OpRndType = FL32;
  OpRounding = RoundingAmount[FL32];
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
  if(other.isInteger()) {
    return *new Float(val + ((Integer*)&other)->val);
  } else if(other.isFloat()) {
    return *new Float(val + ((Float*)&other)->val);
  } else if(other.isDouble()) {
    return *new Double(val + ((Double*)&other)->val);
  }

  return *new BinaryOp((Node *) this, (Node *) &other, ADD);
}

Node &Float::operator-(Node &other) const {
  if(other.isInteger()) {
    return *new Float(val - ((Integer*)&other)->val);
  } else if(other.isFloat()) {
    return *new Float(val - ((Float*)&other)->val);
  } else if(other.isDouble()) {
    return *new Double(val - ((Double*)&other)->val);
  }

  return *new BinaryOp((Node *) this, (Node *) &other, SUB);
}

Node &Float::operator*(Node &other) const {
  if(other.isInteger()) {
    return *new Float(val * ((Integer*)&other)->val);
  } else if(other.isFloat()) {
    return *new Float(val * ((Float*)&other)->val);
  } else if(other.isDouble()) {
    return *new Double(val * ((Double*)&other)->val);
  }

  return *new BinaryOp((Node *) this, (Node *) &other, MUL);
}

Node &Float::operator/(Node &other) const {
  if(other.isInteger()) {
    return *new Float(val / ((Integer*)&other)->val);
  } else if(other.isFloat()) {
    return *new Float(val / ((Float*)&other)->val);
  } else if(other.isDouble()) {
    return *new Double(val / ((Double*)&other)->val);
  }

  return *new BinaryOp((Node *) this, (Node *) &other, DIV);
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

Double::Double(const double val): val(val) {
  type = DOUBLE;
  OpRndType = FL64;
  OpRounding = RoundingAmount[FL64];
}

Double::Double(const ibex::ExprConstant &value): value(&value) {
  type = DOUBLE;
  OpRndType = FL64;
  OpRounding = RoundingAmount[FL64];
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
  if(other.isInteger()) {
    return *new Double(val + ((Integer*)&other)->val);
  } else if(other.isFloat()) {
    return *new Double(val + ((Float*)&other)->val);
  } else if(other.isDouble()) {
    return *new Double(val + ((Double*)&other)->val);
  }

  return *new BinaryOp((Node *) this, (Node *) &other, ADD);
}

Node &Double::operator-(Node &other) const {
  if(other.isInteger()) {
    return *new Double(val - ((Integer*)&other)->val);
  } else if(other.isFloat()) {
    return *new Double(val - ((Float*)&other)->val);
  } else if(other.isDouble()) {
    return *new Double(val - ((Double*)&other)->val);
  }

  return *new BinaryOp((Node *) this, (Node *) &other, SUB);
}

Node &Double::operator*(Node &other) const {
  if(other.isInteger()) {
    return *new Double(val * ((Integer*)&other)->val);
  } else if(other.isFloat()) {
    return *new Double(val * ((Float*)&other)->val);
  } else if(other.isDouble()) {
    return *new Double(val * ((Double*)&other)->val);
  }

  return *new BinaryOp((Node *) this, (Node *) &other, MUL);
}

Node &Double::operator/(Node &other) const {
  if(other.isInteger()) {
    return *new Double(val / ((Integer*)&other)->val);
  } else if(other.isFloat()) {
    return *new Double(val / ((Float*)&other)->val);
  } else if(other.isDouble()) {
    return *new Double(val / ((Double*)&other)->val);
  }

  return *new BinaryOp((Node *) this, (Node *) &other, DIV);
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

FreeVariable::FreeVariable() {
  var = new ibex::Interval(0.1, 100);
  type = FREE_VARIABLE;
}

FreeVariable::FreeVariable(RoundingType rnd_typ) {
  var = new ibex::Interval(0.1, 100);
  type = FREE_VARIABLE;
  OpRndType = rnd_typ;
  OpRounding = RoundingAmount[rnd_typ];
}

FreeVariable::FreeVariable(const ibex::Interval &var, RoundingType rnd_typ): var(&var) {
  type = FREE_VARIABLE;
  OpRndType = rnd_typ;
  OpRounding = RoundingAmount[rnd_typ];
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

  FreeVariable *res;

  if(other.isInteger())
    res = new FreeVariable(*c, INT);
  else if(other.isFloat())
    res = new FreeVariable(*c, FL32);
  else if(other.isDouble())
    res = new FreeVariable(*c, FL64);
  else
    res = new FreeVariable(*c, FL64);

  return *res;
}

Node &FreeVariable::operator-(Node &other) const {
  assert(other.type == FREE_VARIABLE);
  const ibex::Interval *a = var;
  const ibex::Interval *b = ((FreeVariable*)&other)->var;
  const ibex::Interval *c = new ibex::Interval(*a-*b);

  FreeVariable *res;

  if(other.isInteger())
    res = new FreeVariable(*c, INT);
  else if(other.isFloat())
    res = new FreeVariable(*c, FL32);
  else if(other.isDouble())
    res = new FreeVariable(*c, FL64);
  else
    res = new FreeVariable(*c, FL64);

  return *res;
}

Node &FreeVariable::operator*(Node &other) const {
  assert(other.type == FREE_VARIABLE);
  const ibex::Interval *a = var;
  const ibex::Interval *b = ((FreeVariable*)&other)->var;
  const ibex::Interval *c = new ibex::Interval(*a**b);

  FreeVariable *res;

  if(other.isInteger())
    res = new FreeVariable(*c, INT);
  else if(other.isFloat())
    res = new FreeVariable(*c, FL32);
  else if(other.isDouble())
    res = new FreeVariable(*c, FL64);
  else
    res = new FreeVariable(*c, FL64);

  return *res;
}

Node &FreeVariable::operator/(Node &other) const {
  assert(other.type == FREE_VARIABLE);
  const ibex::Interval *a = var;
  const ibex::Interval *b = ((FreeVariable*)&other)->var;
  const ibex::Interval *c = new ibex::Interval(*a/(*b));

  FreeVariable *res;

  if(other.isInteger())
    res = new FreeVariable(*c, INT);
  else if(other.isFloat())
    res = new FreeVariable(*c, FL32);
  else if(other.isDouble())
    res = new FreeVariable(*c, FL64);
  else
    res = new FreeVariable(*c, FL64);

  return *res;
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

VariableNode::VariableNode() {
  type = VARIABLE;
}

VariableNode::VariableNode(RoundingType rnd_typ) {
  type = VARIABLE;
  OpRndType = rnd_typ;
  OpRounding = RoundingAmount[rnd_typ];
}

VariableNode::VariableNode(const ibex::ExprSymbol& variable): variable(&variable) {
  type = VARIABLE;
}

VariableNode::VariableNode(const Node &node) {
  type = VARIABLE;
  parents = node.parents;
  OpRndType = node.OpRndType;
  OpRounding = node.OpRounding;
  
  // Modify the parents of the node to point to this node
  for (auto parent : parents) {
    // switch on parent node type
    switch (parent->type) {
      case UNARY_OP: {
        UnaryOp *unaryOp = (UnaryOp *) parent;
        if (unaryOp->Operand == &node) {
          unaryOp->Operand = this;
        }
        break;
      }
      case BINARY_OP: {
        BinaryOp *binaryOp = (BinaryOp *) parent;
        if (binaryOp->leftOperand == &node) {
          binaryOp->leftOperand = this;
        } else if (binaryOp->rightOperand == &node) {
          binaryOp->rightOperand = this;
        }
        break;
      }
      case TERNARY_OP: {
        TernaryOp *ternaryOp = (TernaryOp *) parent;
        if (ternaryOp->leftOperand == &node) {
          ternaryOp->leftOperand = this;
        } else if (ternaryOp->middleOperand == &node) {
          ternaryOp->middleOperand = this;
        } else if (ternaryOp->rightOperand == &node) {
          ternaryOp->rightOperand = this;
        }
        break;
      }
      default: {
        std::cout << "ERROR: Unknown node type" << std::endl;
        exit(1);
      }
    }
  }

  string prefix = "FR";
  string var_name = prefix + std::to_string(Node::NEW_FREE_VARIABLE_COUNTER++);

  // Set VariableNode members
  variable = &ibex::ExprSymbol::new_(var_name.c_str());
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
  return *new BinaryOp((Node *) this, (Node *) &other, ADD);
}

Node &VariableNode::operator-(Node &other) const {
  return *new BinaryOp((Node *) this, (Node *) &other, SUB);
}

Node &VariableNode::operator*(Node &other) const {
  return *new BinaryOp((Node *) this, (Node *) &other, MUL);
}

Node &VariableNode::operator/(Node &other) const {
  return *new BinaryOp((Node *) this, (Node *) &other, DIV);
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
  switch (op) {
    case NEG:
    case SIN:
    case COS:
    case TAN:
    case SINH:
    case COSH:
    case TANH:
    case ASIN:
    case ACOS:
    case ATAN:
    case LOG:
    case SQRT:
    case EXP:
      OpRndType = Operand->OpRndType;
      OpRounding = Operand->OpRounding;
      break;
  }
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
  return *new BinaryOp((Node *) this, (Node *) &other, ADD);
}

Node &UnaryOp::operator-(Node &other) const {
  return *new BinaryOp((Node *) this, (Node *) &other, SUB);
}

Node &UnaryOp::operator*(Node &other) const {
  return *new BinaryOp((Node *) this, (Node *) &other, MUL);
}

Node &UnaryOp::operator/(Node &other) const {
  return *new BinaryOp((Node *) this, (Node *) &other, DIV);
}

double UnaryOp::getRounding() {
  return OpRounding * OpErrorULPs[op];
}

ibex::ExprNode &UnaryOp::generateSymExpr() {
  switch (op) {
    case NEG: return (ibex::ExprNode &) -(*Operand->getExprNode());
    case SIN: return (ibex::ExprNode &) sin(*Operand->getExprNode());
    case COS: return (ibex::ExprNode &) cos(*Operand->getExprNode());
    case TAN: return (ibex::ExprNode &) tan(*Operand->getExprNode());
    case SINH: return (ibex::ExprNode &) sinh(*Operand->getExprNode());
    case COSH: return (ibex::ExprNode &) cosh(*Operand->getExprNode());
    case TANH: return (ibex::ExprNode &) tanh(*Operand->getExprNode());
    case ASIN: return (ibex::ExprNode &) asin(*Operand->getExprNode());
    case ACOS: return (ibex::ExprNode &) acos(*Operand->getExprNode());
    case ATAN: return (ibex::ExprNode &) atan(*Operand->getExprNode());
    case LOG: return (ibex::ExprNode &) log(*Operand->getExprNode());
    case SQRT: return (ibex::ExprNode &) sqrt(*Operand->getExprNode());
    case EXP: return (ibex::ExprNode &) exp(*Operand->getExprNode());
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
  if (Left->OpRounding != 0.0 && Right->OpRounding != 0.0)
    OpRounding = std::min(Left->OpRounding, Right->OpRounding);
  else if(Left->OpRounding == 0.0)
    OpRounding = Right->OpRounding;
  else
    OpRounding = Left->OpRounding;
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
  return *new BinaryOp((Node *) this, (Node *) &other, ADD);
}

Node &BinaryOp::operator-(Node &other) const {
  return *new BinaryOp((Node *) this, (Node *) &other, SUB);
}

Node &BinaryOp::operator*(Node &other) const {
  return *new BinaryOp((Node *) this, (Node *) &other, MUL);
}

Node &BinaryOp::operator/(Node &other) const {
  return *new BinaryOp((Node *) this, (Node *) &other, DIV);
}

double BinaryOp::getRounding() {
  return OpRounding * OpErrorULPs[op];
}

ibex::ExprNode &BinaryOp::generateSymExpr() {
  switch (op) {
    case ADD: return (ibex::ExprNode &) (*leftOperand->getExprNode() + *rightOperand->getExprNode());
    case SUB: return (ibex::ExprNode &) (*leftOperand->getExprNode() - *rightOperand->getExprNode());
    case MUL: return (ibex::ExprNode &) (*leftOperand->getExprNode() * *rightOperand->getExprNode());
    case DIV: return (ibex::ExprNode &) (*leftOperand->getExprNode() / *rightOperand->getExprNode());
    default:
      std::cout << "ERROR: Unknown operator" << std::endl;
      exit(1);;
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

TernaryOp::TernaryOp(Node* Left, Node* Middle, Node* Right, Op op): leftOperand(Left),
                                                              middleOperand(Middle),
                                                              rightOperand(Right),
                                                              op(op) {
  depth = std::max(Left->depth, std::max(Middle->depth, Right->depth)) + 1;
  type = TERNARY_OP;
  if (Left->OpRounding != 0.0 && Middle->OpRounding != 0.0 && Right->OpRounding != 0.0)
    OpRounding = std::min(Left->OpRounding, std::min(Middle->OpRounding, Right->OpRounding));
  else if(Left->OpRounding == 0.0) {
    if (Middle->OpRounding == 0.0)
      OpRounding = Right->OpRounding;
    else
      OpRounding = Middle->OpRounding;
  }
  else if(Middle->OpRounding == 0.0)
    OpRounding = Right->OpRounding;
  else
    OpRounding = Left->OpRounding;
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
  return (ibex::ExprNode*)expr;
}

bool TernaryOp::operator==(const TernaryOp &other) const {
  return Node::operator==(other) &&
          leftOperand == other.leftOperand &&
          middleOperand == other.middleOperand &&
          rightOperand == other.rightOperand;
}

Node &TernaryOp::operator+(Node &other) const {
  return *new BinaryOp((Node *) this, (Node *) &other, ADD);
}

Node &TernaryOp::operator-(Node &other) const {
  return *new BinaryOp((Node *) this, (Node *) &other, SUB);
}

Node &TernaryOp::operator*(Node &other) const {
  return *new BinaryOp((Node *) this, (Node *) &other, MUL);
}

Node &TernaryOp::operator/(Node &other) const {
  return *new BinaryOp((Node *) this, (Node *) &other, DIV);
}

double TernaryOp::getRounding() {
  // TODO: Add OpRounding for Ternary operations
  return OpRounding;
}

ibex::ExprNode &TernaryOp::generateSymExpr() {
  switch (op) {
    case FMA: return (ibex::ExprNode &) ((*leftOperand->getExprNode() * *middleOperand->getExprNode()) + *rightOperand->getExprNode());
    default:
      std::cout << "ERROR: Unknown operator" << std::endl;
      exit(1);
  }
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
  return *new UnaryOp(&x, Node::NEG);
}

Node &sin(Node &x) {
  return *new UnaryOp(&x, Node::SIN);
}

Node &cos(Node &x) {
  return *new UnaryOp(&x, Node::COS);
}

Node &tan(Node &x) {
  return *new UnaryOp(&x, Node::TAN);
}

Node &sinh(Node &x) {
  return *new UnaryOp(&x, Node::SINH);
}

Node &cosh(Node &x) {
  return *new UnaryOp(&x, Node::COSH);
}

Node &tanh(Node &x) {
  return *new UnaryOp(&x, Node::TANH);
}

Node &asin(Node &x) {
  return *new UnaryOp(&x, Node::ASIN);
}

Node &acos(Node &x) {
  return *new UnaryOp(&x, Node::ACOS);
}

Node &atan(Node &x) {
  return *new UnaryOp(&x, Node::ATAN);
}

Node &log(Node &x) {
  return *new UnaryOp(&x, Node::LOG);
}

Node &sqrt(Node &x) {
  return *new UnaryOp(&x, Node::SQRT);
}

Node &exp(Node &x) {
  return *new UnaryOp(&x, Node::EXP);
}

Node &fma(Node &x, Node &y, Node &z) {
  return x*y+z;
//  return *new TernaryOp(&x, &y, &z, Node::FMA);
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
    return left*right;
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