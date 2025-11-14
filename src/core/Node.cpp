#include "cire/core/Node.h"
#include "cire/interfaces/Logging.h"

#include <algorithm>
#include <iostream>

int Node::NEW_FREE_VARIABLE_COUNTER = 0;
int Node::NODE_COUNTER = 0;

bool Node::isInteger() const { return type == INTEGER; }

bool Node::isFloat() const { return type == FLOAT; }

bool Node::isDouble() const { return type == DOUBLE; }

bool Node::isFreeVariable() const { return type == FREE_VARIABLE; }

bool Node::isVariable() const { return type == VARIABLE; }

bool Node::isUnaryOp() const { return type == UNARY_OP; }

bool Node::isBinaryOp() const { return type == BINARY_OP; }

bool Node::isTernaryOp() const { return type == TERNARY_OP; }

void Node::setRoundingType(Node::RoundingType RndType) { opRoundType = RndType; }

void Node::setRoundingFromType(RoundingType RndType) {
    opRoundType = RndType;
    opRounding = roundingAmount[opRoundType];
}

void Node::setRounding(double opRounding) { this->opRounding = opRounding; }

void Node::setAbsoluteError(const ibex::ExprNode* absErr) { absoluteError = absErr; }

Node::RoundingType Node::getRoundingType() const { return opRoundType; }

void Node::setMetadata(std::unique_ptr<InstructionMetadata> meta) {
    metadata = std::move(meta);
}

InstructionMetadata* Node::getMetadata() const {
    return metadata.get();
}

bool Node::hasMetadata() const {
    return metadata != nullptr;
}

void Node::write(std::ostream& os) const {
    os << "\nID:" << id << "\n";
    os << "\tDepth:" << depth << "\n";

    // Print type
    std::string type_string;
    switch (type) {
        case INTEGER:
            type_string = "INTEGER";
            break;
        case FLOAT:
            type_string = "FLOAT";
            break;
        case DOUBLE:
            type_string = "DOUBLE";
            break;
        case FREE_VARIABLE:
            type_string = "FREE_VARIABLE";
            break;
        case VARIABLE:
            type_string = "VARIABLE";
            break;
        case UNARY_OP:
            type_string = "UNARY_OP";
            break;
        case BINARY_OP:
            type_string = "BINARY_OP";
            break;
        case TERNARY_OP:
            type_string = "TERNARY_OP";
            break;
        default:
            type_string = "DEFAULT";
            break;
    }
    os << "\tType:" << type_string << "\n";
    os << "\tRounding:" << opRounding << "\n";
    os << "\tParents: [\n";
    for (const auto* parent : parents) { os << "\t" << parent->id << ", "; }
    os << "]\n";
}

ibex::ExprNode* Node::getExprNode() const {
    if (logging) {
        logging->critical("Base class getExprNode called");
    } else {
        std::cout << "ERROR: Base class getExprNode called\n";
        exit(1);
    }  // NOLINT
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

Integer::Integer(const int val) : val(val) {
    type = INTEGER;
    opRoundType = INT;
    opRounding = roundingAmount[INT];
}

Integer::Integer(const ibex::ExprConstant& value) : value(&value) {
    type = INTEGER;
    opRoundType = INT;
    opRounding = roundingAmount[INT];
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
    if (other.isInteger()) return *new Integer(val + ((Integer*)&other)->val);
    if (other.isFloat()) return *new Float(val + ((Float*)&other)->val);
    if (other.isDouble()) return *new Double(val + ((Double*)&other)->val);

    return *new BinaryOp((Node*)this, &other, ADD);
}

Node& Integer::operator-(Node& other) const {
    if (other.isInteger()) return *new Integer(val - ((Integer*)&other)->val);
    if (other.isFloat()) return *new Float(val - ((Float*)&other)->val);
    if (other.isDouble()) return *new Double(val - ((Double*)&other)->val);

    return *new BinaryOp((Node*)this, &other, SUB);
}

Node& Integer::operator*(Node& other) const {
    if (other.isInteger()) return *new Integer(val * ((Integer*)&other)->val);
    if (other.isFloat()) return *new Float(val * ((Float*)&other)->val);
    if (other.isDouble()) return *new Double(val * ((Double*)&other)->val);

    return *new BinaryOp((Node*)this, &other, MUL);
}

Node& Integer::operator/(Node& other) const {
    if (other.isInteger()) return *new Integer(val / ((Integer*)&other)->val);
    if (other.isFloat()) return *new Float(val / ((Float*)&other)->val);
    if (other.isDouble()) return *new Double(val / ((Double*)&other)->val);

    return *new BinaryOp((Node*)this, &other, DIV);
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
    type = FLOAT;
    opRoundType = FL32;
    opRounding = roundingAmount[FL32];
}

Float::Float(const ibex::ExprConstant& value) : value(&value) {
    type = FLOAT;
    opRoundType = FL32;
    opRounding = roundingAmount[FL32];
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
    if (other.isInteger()) return *new Float(val + ((Integer*)&other)->val);
    if (other.isFloat()) return *new Float(val + ((Float*)&other)->val);
    if (other.isDouble()) return *new Double(val + ((Double*)&other)->val);

    return *new BinaryOp((Node*)this, &other, ADD);
}

Node& Float::operator-(Node& other) const {
    if (other.isInteger()) return *new Float(val - ((Integer*)&other)->val);
    if (other.isFloat()) return *new Float(val - ((Float*)&other)->val);
    if (other.isDouble()) return *new Double(val - ((Double*)&other)->val);

    return *new BinaryOp((Node*)this, &other, SUB);
}

Node& Float::operator*(Node& other) const {
    if (other.isInteger()) return *new Float(val * ((Integer*)&other)->val);
    if (other.isFloat()) return *new Float(val * ((Float*)&other)->val);
    if (other.isDouble()) return *new Double(val * ((Double*)&other)->val);

    return *new BinaryOp((Node*)this, &other, MUL);
}

Node& Float::operator/(Node& other) const {
    if (other.isInteger()) return *new Float(val / ((Integer*)&other)->val);
    if (other.isFloat()) return *new Float(val / ((Float*)&other)->val);
    if (other.isDouble()) return *new Double(val / ((Double*)&other)->val);

    return *new BinaryOp((Node*)this, &other, DIV);
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
    type = DOUBLE;
    opRoundType = FL64;
    opRounding = roundingAmount[FL64];
}

Double::Double(const ibex::ExprConstant& value) : value(&value) {
    type = DOUBLE;
    opRoundType = FL64;
    opRounding = roundingAmount[FL64];
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
    if (other.isInteger()) return *new Double(val + ((Integer*)&other)->val);
    if (other.isFloat()) return *new Double(val + ((Float*)&other)->val);
    if (other.isDouble()) return *new Double(val + ((Double*)&other)->val);

    return *new BinaryOp((Node*)this, &other, ADD);
}

Node& Double::operator-(Node& other) const {
    if (other.isInteger()) return *new Double(val - ((Integer*)&other)->val);
    if (other.isFloat()) return *new Double(val - ((Float*)&other)->val);
    if (other.isDouble()) return *new Double(val - ((Double*)&other)->val);

    return *new BinaryOp((Node*)this, &other, SUB);
}

Node& Double::operator*(Node& other) const {
    if (other.isInteger()) return *new Double(val * ((Integer*)&other)->val);
    if (other.isFloat()) return *new Double(val * ((Float*)&other)->val);
    if (other.isDouble()) return *new Double(val * ((Double*)&other)->val);

    return *new BinaryOp((Node*)this, &other, MUL);
}

Node& Double::operator/(Node& other) const {
    if (other.isInteger()) return *new Double(val / ((Integer*)&other)->val);
    if (other.isFloat()) return *new Double(val / ((Float*)&other)->val);
    if (other.isDouble()) return *new Double(val / ((Double*)&other)->val);

    return *new BinaryOp((Node*)this, &other, DIV);
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
    type = FREE_VARIABLE;
}

FreeVariable::FreeVariable(RoundingType rnd_typ) {
    var = new ibex::Interval(-1.0, 1.0);
    type = FREE_VARIABLE;
    opRoundType = rnd_typ;
    opRounding = roundingAmount[rnd_typ];
}

FreeVariable::FreeVariable(const ibex::Interval& var, RoundingType rnd_typ) : var(&var) {
    type = FREE_VARIABLE;
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
    assert(other.type == FREE_VARIABLE);
    const ibex::Interval* a = var;
    const ibex::Interval* b = ((FreeVariable*)&other)->var;
    const ibex::Interval* c = new ibex::Interval(*a + *b);

    FreeVariable* res;

    if (other.isInteger()) {
        res = new FreeVariable(*c, INT);
    } else if (other.isFloat()) {
        res = new FreeVariable(*c, FL32);
    } else {
        res = new FreeVariable(*c, FL64);
    }

    return *res;
}

Node& FreeVariable::operator-(Node& other) const {
    assert(other.type == FREE_VARIABLE);
    const ibex::Interval* a = var;
    const ibex::Interval* b = ((FreeVariable*)&other)->var;
    const ibex::Interval* c = new ibex::Interval(*a - *b);

    FreeVariable* res;

    if (other.isInteger()) {
        res = new FreeVariable(*c, INT);
    } else if (other.isFloat()) {
        res = new FreeVariable(*c, FL32);
    } else {
        res = new FreeVariable(*c, FL64);
    }

    return *res;
}

Node& FreeVariable::operator*(Node& other) const {
    assert(other.type == FREE_VARIABLE);
    const ibex::Interval* a = var;
    const ibex::Interval* b = ((FreeVariable*)&other)->var;
    const ibex::Interval* c = new ibex::Interval(*a * *b);

    FreeVariable* res;

    if (other.isInteger()) {
        res = new FreeVariable(*c, INT);
    } else if (other.isFloat()) {
        res = new FreeVariable(*c, FL32);
    } else {
        res = new FreeVariable(*c, FL64);
    }

    return *res;
}

Node& FreeVariable::operator/(Node& other) const {
    assert(other.type == FREE_VARIABLE);
    const ibex::Interval* a = var;
    const ibex::Interval* b = ((FreeVariable*)&other)->var;
    const ibex::Interval* c = new ibex::Interval(*a / (*b));

    FreeVariable* res;

    if (other.isInteger()) {
        res = new FreeVariable(*c, INT);
    } else if (other.isFloat()) {
        res = new FreeVariable(*c, FL32);
    } else {
        res = new FreeVariable(*c, FL64);
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

VariableNode::VariableNode() : variable() { type = VARIABLE; }

VariableNode::VariableNode(RoundingType rnd_typ) : variable() {
    type = VARIABLE;
    opRoundType = rnd_typ;
    opRounding = roundingAmount[rnd_typ];
}

VariableNode::VariableNode(const ibex::ExprSymbol& variable) : variable(&variable) { type = VARIABLE; }

VariableNode::VariableNode(const Node& node) {
    type = VARIABLE;
    parents = node.parents;
    opRoundType = node.opRoundType;
    opRounding = node.opRounding;

    // Modify the parents of the node to point to this node
    for (auto* parent : parents) {
        // switch on parent node type
        switch (parent->type) {
            case UNARY_OP: {
                auto* unaryOp = (UnaryOp*)parent;
                if (unaryOp->operand == &node) { unaryOp->operand = this; }
                break;
            }
            case BINARY_OP: {
                auto* binaryOp = (BinaryOp*)parent;
                if (binaryOp->leftOperand == &node) {
                    binaryOp->leftOperand = this;
                } else if (binaryOp->rightOperand == &node) {
                    binaryOp->rightOperand = this;
                }
                break;
            }
            case TERNARY_OP: {
                auto* ternaryOp = (TernaryOp*)parent;
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

Node& VariableNode::operator+(Node& other) const { return *new BinaryOp((Node*)this, &other, ADD); }

Node& VariableNode::operator-(Node& other) const { return *new BinaryOp((Node*)this, &other, SUB); }

Node& VariableNode::operator*(Node& other) const { return *new BinaryOp((Node*)this, &other, MUL); }

Node& VariableNode::operator/(Node& other) const { return *new BinaryOp((Node*)this, &other, DIV); }

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
UnaryOp::UnaryOp(Node* operand, Op op, RoundingType rnd_typ = CONST) : operand(operand), op(op) {
    depth = operand->depth + 1;
    type = UNARY_OP;
    operand->parents.insert(this);
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
            opRoundType = operand->opRoundType;
            opRounding = operand->opRounding;
            break;
            // We only use rnd_typ for cast instructions as the operator type is not the same as
            // operand type.
        case FPTRUNC:
        case FPEXT:
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

Node& UnaryOp::operator+(Node& other) const { return *new BinaryOp((Node*)this, &other, ADD); }

Node& UnaryOp::operator-(Node& other) const { return *new BinaryOp((Node*)this, &other, SUB); }

Node& UnaryOp::operator*(Node& other) const { return *new BinaryOp((Node*)this, &other, MUL); }

Node& UnaryOp::operator/(Node& other) const { return *new BinaryOp((Node*)this, &other, DIV); }

double UnaryOp::getRounding() { return opRounding * opErrorULPs[op]; }

ibex::ExprNode& UnaryOp::generateSymExpr() {
    switch (op) {
        case NEG:
            return (ibex::ExprNode&)-(*operand->getExprNode());
        case SIN:
            return (ibex::ExprNode&)sin(*operand->getExprNode());
        case COS:
            return (ibex::ExprNode&)cos(*operand->getExprNode());
        case TAN:
            return (ibex::ExprNode&)tan(*operand->getExprNode());
        case SINH:
            return (ibex::ExprNode&)sinh(*operand->getExprNode());
        case COSH:
            return (ibex::ExprNode&)cosh(*operand->getExprNode());
        case TANH:
            return (ibex::ExprNode&)tanh(*operand->getExprNode());
        case ASIN:
            return (ibex::ExprNode&)asin(*operand->getExprNode());
        case ACOS:
            return (ibex::ExprNode&)acos(*operand->getExprNode());
        case ATAN:
            return (ibex::ExprNode&)atan(*operand->getExprNode());
        case LOG:
            return (ibex::ExprNode&)log(*operand->getExprNode());
        case SQRT:
            return (ibex::ExprNode&)sqrt(*operand->getExprNode());
        case EXP:
            return (ibex::ExprNode&)exp(*operand->getExprNode());
        case FPTRUNC:
        case FPEXT:
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

BinaryOp::BinaryOp(Node* Left, Node* Right, Op op) : leftOperand(Left), rightOperand(Right), op(op) {
    depth = std::max(Left->depth, Right->depth) + 1;
    type = BINARY_OP;

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
        case ADD:
            operator_string = "+";
            break;
        case SUB:
            operator_string = "-";
            break;
        case MUL:
            operator_string = "*";
            break;
        case DIV:
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

Node& BinaryOp::operator+(Node& other) const { return *new BinaryOp((Node*)this, &other, ADD); }

Node& BinaryOp::operator-(Node& other) const { return *new BinaryOp((Node*)this, &other, SUB); }

Node& BinaryOp::operator*(Node& other) const { return *new BinaryOp((Node*)this, &other, MUL); }

Node& BinaryOp::operator/(Node& other) const { return *new BinaryOp((Node*)this, &other, DIV); }

double BinaryOp::getRounding() { return opRounding * opErrorULPs[op]; }

ibex::ExprNode& BinaryOp::generateSymExpr() {
    switch (op) {
        case ADD:
            return (ibex::ExprNode&)(*leftOperand->getExprNode() + *rightOperand->getExprNode());
        case SUB:
            return (ibex::ExprNode&)(*leftOperand->getExprNode() - *rightOperand->getExprNode());
        case MUL:
            return (ibex::ExprNode&)(*leftOperand->getExprNode() * *rightOperand->getExprNode());
        case DIV:
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

TernaryOp::TernaryOp(Node* Left, Node* Middle, Node* Right, Op op)
    : leftOperand(Left), middleOperand(Middle), rightOperand(Right), op(op) {
    depth = std::max({Left->depth, Middle->depth, Right->depth}) + 1;
    type = TERNARY_OP;
    if (Left->opRounding != 0.0 && Middle->opRounding != 0.0 && Right->opRounding != 0.0) {
        opRounding = std::max(std::min({Left->opRounding, Middle->opRounding, Right->opRounding}),
                              roundingAmount[FL64]);
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

Node& TernaryOp::operator+(Node& other) const { return *new BinaryOp((Node*)this, &other, ADD); }

Node& TernaryOp::operator-(Node& other) const { return *new BinaryOp((Node*)this, &other, SUB); }

Node& TernaryOp::operator*(Node& other) const { return *new BinaryOp((Node*)this, &other, MUL); }

Node& TernaryOp::operator/(Node& other) const { return *new BinaryOp((Node*)this, &other, DIV); }

double TernaryOp::getRounding() { return opRounding * opErrorULPs[op]; }

ibex::ExprNode& TernaryOp::generateSymExpr() {
    switch (op) {
        case FMA:
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

Node& operator+(Node& x, Node* y) { return x + *y; }

Node& operator-(Node& x, Node* y) { return x - *y; }

Node& operator*(Node& x, Node* y) { return x * *y; }

Node& operator/(Node& x, Node* y) { return x / (*y); }

Node& operator-(Node& x) { return *new UnaryOp(&x, Node::NEG); }

Node& sin(Node& x) { return *new UnaryOp(&x, Node::SIN); }

Node& cos(Node& x) { return *new UnaryOp(&x, Node::COS); }

Node& tan(Node& x) { return *new UnaryOp(&x, Node::TAN); }

Node& sinh(Node& x) { return *new UnaryOp(&x, Node::SINH); }

Node& cosh(Node& x) { return *new UnaryOp(&x, Node::COSH); }

Node& tanh(Node& x) { return *new UnaryOp(&x, Node::TANH); }

Node& asin(Node& x) { return *new UnaryOp(&x, Node::ASIN); }

Node& acos(Node& x) { return *new UnaryOp(&x, Node::ACOS); }

Node& atan(Node& x) { return *new UnaryOp(&x, Node::ATAN); }

Node& log(Node& x) { return *new UnaryOp(&x, Node::LOG); }

Node& sqrt(Node& x) { return *new UnaryOp(&x, Node::SQRT); }

Node& exp(Node& x) { return *new UnaryOp(&x, Node::EXP); }

Node& fptrunc(Node& x, Node::RoundingType rnd_typ) { return *new UnaryOp(&x, Node::FPTRUNC, rnd_typ); }

Node& fpext(Node& x, Node::RoundingType rnd_typ) { return *new UnaryOp(&x, Node::FPEXT, rnd_typ); }

Node& fma(Node& x, Node& y, Node& z) {
    //  return x*y+z;
    return *new TernaryOp(&x, &y, &z, Node::FMA);
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
