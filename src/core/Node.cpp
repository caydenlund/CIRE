#include "cire/core/Node.hpp"
#include "cire/interfaces/Logging.h"

#include <algorithm>
#include <iostream>

using ir::Node;
using Type = ir::Node::Type;
using OpType = ir::Node::OpType;
using RoundingType = ir::Node::RoundingType;

using ir::BinaryOp;
using ir::Double;
using ir::Float;
using ir::FreeVariable;
using ir::Integer;
using ir::TernaryOp;
using ir::UnaryOp;
using ir::VariableNode;

int Node::NEW_FREE_VARIABLE_COUNTER = 0;
int Node::NODE_COUNTER = 0;

bool Node::isInteger() const { return type == Type::INTEGER; }

bool Node::isFloat() const { return type == Type::FLOAT; }

bool Node::isDouble() const { return type == Type::DOUBLE; }

bool Node::isFreeVariable() const { return type == Type::FREE_VARIABLE; }

bool Node::isVariable() const { return type == Type::VARIABLE; }

bool Node::isUnaryOp() const { return type == Type::UNARY_OP; }

bool Node::isBinaryOp() const { return type == Type::BINARY_OP; }

bool Node::isTernaryOp() const { return type == Type::TERNARY_OP; }

void Node::setRoundingType(RoundingType RndType) { opRoundType = RndType; }

void Node::setRoundingFromType(RoundingType RndType) {
    opRoundType = RndType;
    opRounding = roundingAmount[opRoundType];
}

void Node::setRounding(double opRounding) { this->opRounding = opRounding; }

void Node::setAbsoluteError(const ibex::ExprNode* absErr) { absoluteError = absErr; }

RoundingType Node::getRoundingType() const { return opRoundType; }

void Node::setMetadata(std::unique_ptr<InstructionMetadata> meta) { metadata = std::move(meta); }

InstructionMetadata* Node::getMetadata() const { return metadata.get(); }

bool Node::hasMetadata() const { return metadata != nullptr; }

void Node::write(std::ostream& os) const {
    os << "\nID:" << id << "\n";
    os << "\tDepth:" << depth << "\n";

    // Print type
    std::string type_string;
    switch (type) {
        case Type::INTEGER:
            type_string = "INTEGER";
            break;
        case Type::FLOAT:
            type_string = "FLOAT";
            break;
        case Type::DOUBLE:
            type_string = "DOUBLE";
            break;
        case Type::FREE_VARIABLE:
            type_string = "FREE_VARIABLE";
            break;
        case Type::VARIABLE:
            type_string = "VARIABLE";
            break;
        case Type::UNARY_OP:
            type_string = "UNARY_OP";
            break;
        case Type::BINARY_OP:
            type_string = "BINARY_OP";
            break;
        case Type::TERNARY_OP:
            type_string = "TERNARY_OP";
            break;
        default:
            type_string = "DEFAULT";
            break;
    }
    os << "\tType:" << type_string << "\n";
    os << "\tRounding:" << opRounding << "\n";
    os << "\tParents: [\n";
    for (const auto* parent : parents) os << "\t" << parent->id << ", ";
    os << "]\n";
}

ibex::ExprNode* Node::getExprNode() const {
    logging->critical("Base class getExprNode called");
    exit(1);  // NOLINT
}

bool Node::operator==(const Node& other) const {
    return depth == other.depth && type == other.type && opRounding == other.opRounding;
}

Node& Node::operator+(Node&) const {
    std::cout << "ERROR: Base class operator+ called\n";
    exit(1);  // NOLINT
}

Node& Node::operator-(Node&) const {
    std::cout << "ERROR: Base class operator- called\n";
    exit(1);  // NOLINT
}

Node& Node::operator*(Node&) const {
    std::cout << "ERROR: Base class operator* called\n";
    exit(1);  // NOLINT
}

Node& Node::operator/(Node&) const {
    std::cout << "ERROR: Base class operator/ called\n";
    exit(1);  // NOLINT
}

double Node::getRounding() { return opRounding; }

ibex::ExprNode& Node::getAbsoluteError() { return *getExprNode(); }

ibex::ExprNode& Node::generateSymExpr() {
    std::cout << "ERROR: Base class generateSymExpr called. Base class does not "
                 "have an Ibex Expression field"
              << "\n";
    exit(1);  // NOLINT
}

Node* Node::getChildNode(int) const {
    std::cout << "ERROR: Base class getChildNode called. Base class does not "
                 "have child nodes"
              << "\n";
    exit(1);  // NOLINT
}

ir::Integer::Integer(const int val) : val(val) {
    type = Type::INTEGER;
    opRoundType = RoundingType::INT;
    opRounding = roundingAmount[RoundingType::INT];
}

Integer::Integer(const ibex::ExprConstant& value) : value(&value) {
    type = Type::INTEGER;
    opRoundType = RoundingType::INT;
    opRounding = roundingAmount[RoundingType::INT];
}

void Integer::write(std::ostream& os) const {
    // Call parent class operator
    Node::write(os);

    // Print remaining data
    os << "\tValue:" << value << "\n";
}

ibex::ExprNode* Integer::getExprNode() const { return (ibex::ExprNode*)value; }

bool Integer::operator==(const Integer& other) const { return Node::operator==(other) && value == other.value; }

Node& Integer::operator+(Node& other) const {
    if (other.isInteger()) return *new Integer(val + dynamic_cast<Integer*>(&other)->val);
    if (other.isFloat()) return *new Float(float(val) + dynamic_cast<Float*>(&other)->val);
    if (other.isDouble()) return *new Double(val + dynamic_cast<Double*>(&other)->val);

    return *new BinaryOp((Node*)this, &other, OpType::ADD);
}

Node& Integer::operator-(Node& other) const {
    if (other.isInteger()) return *new Integer(val - dynamic_cast<Integer*>(&other)->val);
    if (other.isFloat()) return *new Float(float(val) - dynamic_cast<Float*>(&other)->val);
    if (other.isDouble()) return *new Double(val - dynamic_cast<Double*>(&other)->val);

    return *new BinaryOp((Node*)this, &other, OpType::SUB);
}

Node& Integer::operator*(Node& other) const {
    if (other.isInteger()) return *new Integer(val * dynamic_cast<Integer*>(&other)->val);
    if (other.isFloat()) return *new Float(float(val) * dynamic_cast<Float*>(&other)->val);
    if (other.isDouble()) return *new Double(val * dynamic_cast<Double*>(&other)->val);

    return *new BinaryOp((Node*)this, &other, OpType::MUL);
}

Node& Integer::operator/(Node& other) const {
    if (other.isInteger()) return *new Integer(val / dynamic_cast<Integer*>(&other)->val);
    if (other.isFloat()) return *new Float(float(val) / dynamic_cast<Float*>(&other)->val);
    if (other.isDouble()) return *new Double(val / dynamic_cast<Double*>(&other)->val);

    return *new BinaryOp((Node*)this, &other, OpType::DIV);
}

ibex::ExprNode& Integer::generateSymExpr() {
    assert(value != nullptr
           && "ERROR: ibex::ExprConstant with Integer value should have been assigned while "
              "parsing/"
              "node creation\n");
    return *getExprNode();
}

Node* Integer::getChildNode(int) const {
    std::cout << "ERROR: Integer Class does not have child nodes\n";
    exit(1);  // NOLINT
}

Float::Float(const float val) : val(val) {
    type = Type::FLOAT;
    opRoundType = RoundingType::FL32;
    opRounding = roundingAmount[RoundingType::FL32];
}

Float::Float(const ibex::ExprConstant& value) : value(&value) {
    type = Type::FLOAT;
    opRoundType = RoundingType::FL32;
    opRounding = roundingAmount[RoundingType::FL32];
}

void Float::write(std::ostream& os) const {
    // Call parent class operator
    Node::write(os);

    // Print remaining data
    os << "\tValue:" << value << "\n";
}

ibex::ExprNode* Float::getExprNode() const { return (ibex::ExprNode*)value; }

bool Float::operator==(const Float& other) const { return Node::operator==(other) && value == other.value; }

Node& Float::operator+(Node& other) const {
    if (other.isInteger()) return *new Float(val + float(dynamic_cast<Integer*>(&other)->val));
    if (other.isFloat()) return *new Float(val + dynamic_cast<Float*>(&other)->val);
    if (other.isDouble()) return *new Double(val + dynamic_cast<Double*>(&other)->val);

    return *new BinaryOp((Node*)this, &other, OpType::ADD);
}

Node& Float::operator-(Node& other) const {
    if (other.isInteger()) return *new Float(val - float(dynamic_cast<Integer*>(&other)->val));
    if (other.isFloat()) return *new Float(val - dynamic_cast<Float*>(&other)->val);
    if (other.isDouble()) return *new Double(val - dynamic_cast<Double*>(&other)->val);

    return *new BinaryOp((Node*)this, &other, OpType::SUB);
}

Node& Float::operator*(Node& other) const {
    if (other.isInteger()) return *new Float(val * float(dynamic_cast<Integer*>(&other)->val));
    if (other.isFloat()) return *new Float(val * dynamic_cast<Float*>(&other)->val);
    if (other.isDouble()) return *new Double(val * dynamic_cast<Double*>(&other)->val);

    return *new BinaryOp((Node*)this, &other, OpType::MUL);
}

Node& Float::operator/(Node& other) const {
    if (other.isInteger()) return *new Float(val / float(dynamic_cast<Integer*>(&other)->val));
    if (other.isFloat()) return *new Float(val / dynamic_cast<Float*>(&other)->val);
    if (other.isDouble()) return *new Double(val / dynamic_cast<Double*>(&other)->val);

    return *new BinaryOp((Node*)this, &other, OpType::DIV);
}

ibex::ExprNode& Float::getAbsoluteError() { return (ibex::ExprNode&)*absoluteError; }

ibex::ExprNode& Float::generateSymExpr() {
    assert(value != nullptr
           && "ERROR: ibex::ExprConstant with Float value should have been assigned while parsing/"
              "node creation\n");
    return *getExprNode();
}

Node* Float::getChildNode(int) const {
    std::cout << "ERROR: Float Class does not have child nodes\n";
    exit(1);  // NOLINT
}

Double::Double(const double val) : val(val) {
    type = Type::DOUBLE;
    opRoundType = RoundingType::FL64;
    opRounding = roundingAmount[RoundingType::FL64];
}

Double::Double(const ibex::ExprConstant& value) : value(&value) {
    type = Type::DOUBLE;
    opRoundType = RoundingType::FL64;
    opRounding = roundingAmount[RoundingType::FL64];
}

void Double::write(std::ostream& os) const {
    // Call parent class operator
    Node::write(os);

    // Print remaining data
    os << "\tValue:" << value << "\n";
}

ibex::ExprNode* Double::getExprNode() const { return (ibex::ExprNode*)value; }

bool Double::operator==(const Double& other) const { return Node::operator==(other) && value == other.value; }

Node& Double::operator+(Node& other) const {
    if (other.isInteger()) return *new Double(val + dynamic_cast<Integer*>(&other)->val);
    if (other.isFloat()) return *new Double(val + dynamic_cast<Float*>(&other)->val);
    if (other.isDouble()) return *new Double(val + dynamic_cast<Double*>(&other)->val);

    return *new BinaryOp((Node*)this, &other, OpType::ADD);
}

Node& Double::operator-(Node& other) const {
    if (other.isInteger()) return *new Double(val - dynamic_cast<Integer*>(&other)->val);
    if (other.isFloat()) return *new Double(val - dynamic_cast<Float*>(&other)->val);
    if (other.isDouble()) return *new Double(val - dynamic_cast<Double*>(&other)->val);

    return *new BinaryOp((Node*)this, &other, OpType::SUB);
}

Node& Double::operator*(Node& other) const {
    if (other.isInteger()) return *new Double(val * dynamic_cast<Integer*>(&other)->val);
    if (other.isFloat()) return *new Double(val * dynamic_cast<Float*>(&other)->val);
    if (other.isDouble()) return *new Double(val * dynamic_cast<Double*>(&other)->val);

    return *new BinaryOp((Node*)this, &other, OpType::MUL);
}

Node& Double::operator/(Node& other) const {
    if (other.isInteger()) return *new Double(val / dynamic_cast<Integer*>(&other)->val);
    if (other.isFloat()) return *new Double(val / dynamic_cast<Float*>(&other)->val);
    if (other.isDouble()) return *new Double(val / dynamic_cast<Double*>(&other)->val);

    return *new BinaryOp((Node*)this, &other, OpType::DIV);
}

ibex::ExprNode& Double::getAbsoluteError() { return (ibex::ExprNode&)*absoluteError; }

ibex::ExprNode& Double::generateSymExpr() {
    assert(value != nullptr
           && "ERROR: ibex::ExprConstant with Double value should have been assigned while parsing/"
              "node creation\n");
    return *getExprNode();
}

Node* Double::getChildNode(int) const {
    std::cout << "ERROR: Double Class does not have child nodes\n";
    exit(1);  // NOLINT
}

FreeVariable::FreeVariable() {
    var = new ibex::Interval(-1.0, 1.0);
    type = Type::FREE_VARIABLE;
}

FreeVariable::FreeVariable(RoundingType rnd_typ) {
    var = new ibex::Interval(-1.0, 1.0);
    type = Type::FREE_VARIABLE;
    opRoundType = rnd_typ;
    opRounding = roundingAmount[rnd_typ];
}

FreeVariable::FreeVariable(const ibex::Interval& var, RoundingType rnd_typ) : var(&var) {
    type = Type::FREE_VARIABLE;
    opRoundType = rnd_typ;
    opRounding = roundingAmount[rnd_typ];
}

void FreeVariable::write(std::ostream& os) const {
    // Call parent class operator
    Node::write(os);

    // Print remaining data
    os << "\tValue:" << *var << "\n";
}

Node& FreeVariable::operator+(Node& other) const {
    assert(other.type == Type::FREE_VARIABLE);
    const ibex::Interval* a = var;
    const ibex::Interval* b = dynamic_cast<FreeVariable*>(&other)->var;
    const ibex::Interval* c = new ibex::Interval(*a + *b);

    FreeVariable* res;

    if (other.isInteger()) {
        res = new FreeVariable(*c, RoundingType::INT);
    } else if (other.isFloat()) {
        res = new FreeVariable(*c, RoundingType::FL32);
    } else {
        res = new FreeVariable(*c, RoundingType::FL64);
    }

    return *res;
}

Node& FreeVariable::operator-(Node& other) const {
    assert(other.type == Type::FREE_VARIABLE);
    const ibex::Interval* a = var;
    const ibex::Interval* b = dynamic_cast<FreeVariable*>(&other)->var;
    const ibex::Interval* c = new ibex::Interval(*a - *b);

    FreeVariable* res;

    if (other.isInteger()) {
        res = new FreeVariable(*c, RoundingType::INT);
    } else if (other.isFloat()) {
        res = new FreeVariable(*c, RoundingType::FL32);
    } else {
        res = new FreeVariable(*c, RoundingType::FL64);
    }

    return *res;
}

Node& FreeVariable::operator*(Node& other) const {
    assert(other.type == Type::FREE_VARIABLE);
    const ibex::Interval* a = var;
    const ibex::Interval* b = dynamic_cast<FreeVariable*>(&other)->var;
    const ibex::Interval* c = new ibex::Interval(*a * *b);

    FreeVariable* res;

    if (other.isInteger()) {
        res = new FreeVariable(*c, RoundingType::INT);
    } else if (other.isFloat()) {
        res = new FreeVariable(*c, RoundingType::FL32);
    } else {
        res = new FreeVariable(*c, RoundingType::FL64);
    }

    return *res;
}

Node& FreeVariable::operator/(Node& other) const {
    assert(other.type == Type::FREE_VARIABLE);
    const ibex::Interval* a = var;
    const ibex::Interval* b = dynamic_cast<FreeVariable*>(&other)->var;
    const ibex::Interval* c = new ibex::Interval(*a / (*b));

    FreeVariable* res;

    if (other.isInteger()) {
        res = new FreeVariable(*c, RoundingType::INT);
    } else if (other.isFloat()) {
        res = new FreeVariable(*c, RoundingType::FL32);
    } else {
        res = new FreeVariable(*c, RoundingType::FL64);
    }

    return *res;
}

ibex::ExprNode& FreeVariable::getAbsoluteError() { return (ibex::ExprNode&)*absoluteError; }

ibex::ExprNode& FreeVariable::generateSymExpr() {
    assert(var != nullptr
           && "ERROR: ibex::Interval with Interval value should have been assigned while parsing/"
              "node creation\n");
    exit(1);  // NOLINT
}

Node* FreeVariable::getChildNode(int) const {
    std::cout << "ERROR: FreeVariable Class does not have child nodes\n";
    exit(1);  // NOLINT
}

VariableNode::VariableNode() : variable() { type = Type::VARIABLE; }

VariableNode::VariableNode(RoundingType rnd_typ) : variable() {
    type = Type::VARIABLE;
    opRoundType = rnd_typ;
    opRounding = roundingAmount[rnd_typ];
}

VariableNode::VariableNode(const ibex::ExprSymbol& variable) : variable(&variable) { type = Type::VARIABLE; }

VariableNode::VariableNode(const Node& node) {
    type = Type::VARIABLE;
    parents = node.parents;
    opRoundType = node.opRoundType;
    opRounding = node.opRounding;

    // Modify the parents of the node to point to this node
    for (auto* parent : parents) {
        // switch on parent node type
        switch (parent->type) {
            case Type::UNARY_OP: {
                auto* unaryOp = dynamic_cast<UnaryOp*>(parent);
                if (unaryOp->operand == &node) unaryOp->operand = this;
                break;
            }
            case Type::BINARY_OP: {
                auto* binaryOp = dynamic_cast<BinaryOp*>(parent);
                if (binaryOp->leftOperand == &node) {
                    binaryOp->leftOperand = this;
                } else if (binaryOp->rightOperand == &node) {
                    binaryOp->rightOperand = this;
                }
                break;
            }
            case Type::TERNARY_OP: {
                auto* ternaryOp = dynamic_cast<TernaryOp*>(parent);
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
                std::cout << "ERROR: Unknown node type\n";
                exit(1);  // NOLINT
            }
        }
    }

    string prefix = "FR";
    string var_name = prefix + std::to_string(Node::NEW_FREE_VARIABLE_COUNTER++);

    // Set VariableNode members
    variable = &ibex::ExprSymbol::new_(var_name.c_str());
}

void VariableNode::write(std::ostream& os) const {
    // Call parent class operator
    Node::write(os);

    // Print remaining data
    os << "\tVariable:" << *variable << "\n";
}

ibex::ExprNode* VariableNode::getExprNode() const { return (ibex::ExprNode*)variable; }

bool VariableNode::operator==(const VariableNode& other) const {
    return Node::operator==(other) && variable == other.variable;
}

Node& VariableNode::operator+(Node& other) const { return *new BinaryOp((Node*)this, &other, OpType::ADD); }

Node& VariableNode::operator-(Node& other) const { return *new BinaryOp((Node*)this, &other, OpType::SUB); }

Node& VariableNode::operator*(Node& other) const { return *new BinaryOp((Node*)this, &other, OpType::MUL); }

Node& VariableNode::operator/(Node& other) const { return *new BinaryOp((Node*)this, &other, OpType::DIV); }

ibex::ExprNode& VariableNode::getAbsoluteError() { return (ibex::ExprNode&)*absoluteError; }

ibex::ExprNode& VariableNode::generateSymExpr() {
    assert(variable != nullptr
           && "ERROR: ibex::ExprSymbol with string literal should have been assigned while parsing/"
              "node creation\n");
    return *getExprNode();
}

Node* VariableNode::getChildNode(int) const {
    std::cout << "ERROR: VariableNode Class does not have child nodes\n";
    exit(1);  // NOLINT
}

// We assume the opRoundType of operator is the same as opRoundType of the operands except in the case
// of FPTRUNC and FPEXT where we explicitly pass the rounding type in rnd_typ
UnaryOp::UnaryOp(Node* operand, OpType op, RoundingType rnd_typ = RoundingType::CONST) : operand(operand), op(op) {
    depth = operand->depth + 1;
    type = Type::UNARY_OP;
    operand->parents.insert(this);
    switch (op) {
        case OpType::NEG:
        case OpType::SIN:
        case OpType::COS:
        case OpType::TAN:
        case OpType::SINH:
        case OpType::COSH:
        case OpType::TANH:
        case OpType::ASIN:
        case OpType::ACOS:
        case OpType::ATAN:
        case OpType::LOG:
        case OpType::SQRT:
        case OpType::EXP:
            opRoundType = operand->opRoundType;
            opRounding = operand->opRounding;
            break;
            // We only use rnd_typ for cast instructions as the operator type is not the same as
            // operand type.
        case OpType::FPTRUNC:
        case OpType::FPEXT:
            opRoundType = rnd_typ;
            opRounding = roundingAmount[rnd_typ];
            break;
        default:
            break;
    }
}

void UnaryOp::write(std::ostream& os) const {
    // Call parent class operator
    Node::write(os);

    // Print remaining data
    os << "\tOperand: [" << *operand << "]\n";
}

ibex::ExprNode* UnaryOp::getExprNode() const { return (ibex::ExprNode*)expr; }

bool UnaryOp::operator==(const UnaryOp& other) const {
    return Node::operator==(other) && operand == other.operand && op == other.op;
}

Node& UnaryOp::operator+(Node& other) const { return *new BinaryOp((Node*)this, &other, OpType::ADD); }

Node& UnaryOp::operator-(Node& other) const { return *new BinaryOp((Node*)this, &other, OpType::SUB); }

Node& UnaryOp::operator*(Node& other) const { return *new BinaryOp((Node*)this, &other, OpType::MUL); }

Node& UnaryOp::operator/(Node& other) const { return *new BinaryOp((Node*)this, &other, OpType::DIV); }

double UnaryOp::getRounding() { return opRounding * opErrorULPs[op]; }

ibex::ExprNode& UnaryOp::generateSymExpr() {
    switch (op) {
        case OpType::NEG:
            return (ibex::ExprNode&)-(*operand->getExprNode());
        case OpType::SIN:
            return (ibex::ExprNode&)sin(*operand->getExprNode());
        case OpType::COS:
            return (ibex::ExprNode&)cos(*operand->getExprNode());
        case OpType::TAN:
            return (ibex::ExprNode&)tan(*operand->getExprNode());
        case OpType::SINH:
            return (ibex::ExprNode&)sinh(*operand->getExprNode());
        case OpType::COSH:
            return (ibex::ExprNode&)cosh(*operand->getExprNode());
        case OpType::TANH:
            return (ibex::ExprNode&)tanh(*operand->getExprNode());
        case OpType::ASIN:
            return (ibex::ExprNode&)asin(*operand->getExprNode());
        case OpType::ACOS:
            return (ibex::ExprNode&)acos(*operand->getExprNode());
        case OpType::ATAN:
            return (ibex::ExprNode&)atan(*operand->getExprNode());
        case OpType::LOG:
            return (ibex::ExprNode&)log(*operand->getExprNode());
        case OpType::SQRT:
            return (ibex::ExprNode&)sqrt(*operand->getExprNode());
        case OpType::EXP:
            return (ibex::ExprNode&)exp(*operand->getExprNode());
        case OpType::FPTRUNC:
        case OpType::FPEXT:
            return *operand->getExprNode();
        default:
            std::cout << "ERROR: Unknown operator\n";
            exit(1);  // NOLINT
    }
}

Node* UnaryOp::getChildNode(int index) const {
    if (index == 0) return operand;

    std::cout << "ERROR: UnaryOp Class has only one child node\n";
    exit(1);  // NOLINT
}

BinaryOp::BinaryOp(Node* Left, Node* Right, OpType op) : leftOperand(Left), rightOperand(Right), op(op) {
    depth = std::max(Left->depth, Right->depth) + 1;
    type = Type::BINARY_OP;

    if (Left->opRounding != 0.0 && Right->opRounding != 0.0) {
        opRounding = std::min(Left->opRounding, Right->opRounding);
    } else if (Left->opRounding == 0.0) {
        opRounding = Right->opRounding;
    } else {
        opRounding = Left->opRounding;
    }

    leftOperand->parents.insert(this);
    rightOperand->parents.insert(this);
}

void BinaryOp::write(std::ostream& os) const {
    // Call parent class operator
    Node::write(os);

    // Print remaining data
    os << "\tLeft Operand: [" << *leftOperand << "]\n";
    // Print operator
    std::string operator_string;
    switch (op) {
        case OpType::ADD:
            operator_string = "+";
            break;
        case OpType::SUB:
            operator_string = "-";
            break;
        case OpType::MUL:
            operator_string = "*";
            break;
        case OpType::DIV:
            operator_string = "/";
            break;
        default:
            operator_string = "Error: Unknown operator accepted";
            break;
    }

    os << "\tOperator: " << operator_string << "\n";
    os << "\tRight Operand: [" << *rightOperand << "]\n";
}

bool BinaryOp::operator==(const BinaryOp& other) const {
    return Node::operator==(other) && leftOperand == other.leftOperand && rightOperand == other.rightOperand
        && op == other.op;
}

ibex::ExprNode* BinaryOp::getExprNode() const { return (ibex::ExprNode*)expr; }

Node& BinaryOp::operator+(Node& other) const { return *new BinaryOp((Node*)this, &other, OpType::ADD); }

Node& BinaryOp::operator-(Node& other) const { return *new BinaryOp((Node*)this, &other, OpType::SUB); }

Node& BinaryOp::operator*(Node& other) const { return *new BinaryOp((Node*)this, &other, OpType::MUL); }

Node& BinaryOp::operator/(Node& other) const { return *new BinaryOp((Node*)this, &other, OpType::DIV); }

double BinaryOp::getRounding() { return opRounding * opErrorULPs[op]; }

ibex::ExprNode& BinaryOp::generateSymExpr() {
    switch (op) {
        case OpType::ADD:
            return (ibex::ExprNode&)(*leftOperand->getExprNode() + *rightOperand->getExprNode());
        case OpType::SUB:
            return (ibex::ExprNode&)(*leftOperand->getExprNode() - *rightOperand->getExprNode());
        case OpType::MUL:
            return (ibex::ExprNode&)(*leftOperand->getExprNode() * *rightOperand->getExprNode());
        case OpType::DIV:
            return (ibex::ExprNode&)(*leftOperand->getExprNode() / *rightOperand->getExprNode());
        default:
            std::cout << "ERROR: Unknown operator\n";
            exit(1);  // NOLINT
            ;
    }
}

Node* BinaryOp::getChildNode(int index) const {
    if (index == 0) return leftOperand;
    if (index == 1) return rightOperand;

    std::cout << "ERROR: BinaryOp Class has only two child nodes\n";
    exit(1);  // NOLINT
}

TernaryOp::TernaryOp(Node* Left, Node* Middle, Node* Right, OpType op)
    : leftOperand(Left), middleOperand(Middle), rightOperand(Right), op(op) {
    depth = std::max({Left->depth, Middle->depth, Right->depth}) + 1;
    type = Type::TERNARY_OP;
    if (Left->opRounding != 0.0 && Middle->opRounding != 0.0 && Right->opRounding != 0.0) {
        opRounding = std::max(std::min({Left->opRounding, Middle->opRounding, Right->opRounding}),
                              roundingAmount[RoundingType::FL64]);
    } else if (Left->opRounding == 0.0) {
        if (Middle->opRounding == 0.0) {
            opRounding = Right->opRounding;
        } else {
            opRounding = Middle->opRounding;
        }
    } else if (Middle->opRounding == 0.0) {
        opRounding = Right->opRounding;
    } else {
        opRounding = Left->opRounding;
    }
    leftOperand->parents.insert(this);
    middleOperand->parents.insert(this);
    rightOperand->parents.insert(this);
}

void TernaryOp::write(std::ostream& os) const {
    // Call parent class operator
    Node::write(os);

    // Print remaining data
    os << "\tLeft Operand: [" << *leftOperand << "]\n";
    os << "\tMiddle Operand: [" << *middleOperand << "]\n";
    os << "\tRight Operand: [" << *rightOperand << "]\n";
}

ibex::ExprNode* TernaryOp::getExprNode() const { return (ibex::ExprNode*)expr; }

bool TernaryOp::operator==(const TernaryOp& other) const {
    return Node::operator==(other) && leftOperand == other.leftOperand && middleOperand == other.middleOperand
        && rightOperand == other.rightOperand;
}

Node& TernaryOp::operator+(Node& other) const { return *new BinaryOp((Node*)this, &other, OpType::ADD); }

Node& TernaryOp::operator-(Node& other) const { return *new BinaryOp((Node*)this, &other, OpType::SUB); }

Node& TernaryOp::operator*(Node& other) const { return *new BinaryOp((Node*)this, &other, OpType::MUL); }

Node& TernaryOp::operator/(Node& other) const { return *new BinaryOp((Node*)this, &other, OpType::DIV); }

double TernaryOp::getRounding() { return opRounding * opErrorULPs[op]; }

ibex::ExprNode& TernaryOp::generateSymExpr() {
    switch (op) {
        case OpType::FMA:
            return (ibex::ExprNode&)((*leftOperand->getExprNode() * *middleOperand->getExprNode())
                                     + *rightOperand->getExprNode());
        default:
            std::cout << "ERROR: Unknown operator\n";
            exit(1);  // NOLINT
    }
}

Node* TernaryOp::getChildNode(int index) const {
    if (index == 0) return leftOperand;
    if (index == 1) return middleOperand;
    if (index == 2) return rightOperand;

    std::cout << "ERROR: TernaryOp Class has only three child nodes\n";
    exit(1);  // NOLINT
}

std::ostream& operator<<(std::ostream& os, const Node& node) {
    node.write(os);
    return os;
}

namespace ir {
    Node& operator+(Node& x, Node* y) { return x + *y; }

    Node& operator-(Node& x, Node* y) { return x - *y; }

    Node& operator*(Node& x, Node* y) { return x * *y; }

    Node& operator/(Node& x, Node* y) { return x / (*y); }
}  // namespace ir

Node& operator-(Node& x) { return *new UnaryOp(&x, OpType::NEG); }

Node& sin(Node& x) { return *new UnaryOp(&x, OpType::SIN); }

Node& cos(Node& x) { return *new UnaryOp(&x, OpType::COS); }

Node& tan(Node& x) { return *new UnaryOp(&x, OpType::TAN); }

Node& sinh(Node& x) { return *new UnaryOp(&x, OpType::SINH); }

Node& cosh(Node& x) { return *new UnaryOp(&x, OpType::COSH); }

Node& tanh(Node& x) { return *new UnaryOp(&x, OpType::TANH); }

Node& asin(Node& x) { return *new UnaryOp(&x, OpType::ASIN); }

Node& acos(Node& x) { return *new UnaryOp(&x, OpType::ACOS); }

Node& atan(Node& x) { return *new UnaryOp(&x, OpType::ATAN); }

Node& log(Node& x) { return *new UnaryOp(&x, OpType::LOG); }

Node& sqrt(Node& x) { return *new UnaryOp(&x, OpType::SQRT); }

Node& exp(Node& x) { return *new UnaryOp(&x, OpType::EXP); }

Node& fptrunc(Node& x, RoundingType rnd_typ) { return *new UnaryOp(&x, OpType::FPTRUNC, rnd_typ); }

Node& fpext(Node& x, RoundingType rnd_typ) { return *new UnaryOp(&x, OpType::FPEXT, rnd_typ); }

Node& fma(Node& x, Node& y, Node& z) {
    //  return x*y+z;
    return *new TernaryOp(&x, &y, &z, OpType::FMA);
}

const ibex::ExprNode& product(const ibex::ExprNode& left, const ibex::ExprNode& right) {
    if (left == ibex::ExprConstant::new_scalar(1.0) && right == ibex::ExprConstant::new_scalar(1.0))
        return ibex::ExprConstant::new_scalar(1.0);
    if (left == ibex::ExprConstant::new_scalar(1.0)) return right;
    if (left == ibex::ExprConstant::new_scalar(-1.0)) return -right;
    if (right == ibex::ExprConstant::new_scalar(1.0)) return left;
    if (right == ibex::ExprConstant::new_scalar(-1.0)) return -left;

    return left * right;
}

const ibex::ExprNode& product(const ibex::ExprNode& left, double right) {
    if (left == ibex::ExprConstant::new_scalar(1.0) && right == 1.0) return ibex::ExprConstant::new_scalar(1.0);
    if (left == ibex::ExprConstant::new_scalar(1.0)) return ibex::ExprConstant::new_scalar(right);
    if (left == ibex::ExprConstant::new_scalar(-1.0)) return ibex::ExprConstant::new_scalar(-right);
    if (right == 1.0) return left;
    if (right == -1.0) return -left;

    return left * ibex::ExprConstant::new_scalar(right);
}
