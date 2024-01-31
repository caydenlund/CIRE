#include "../include/IBEXInterface.h"

IBEXInterface::IBEXInterface(ibex::IntervalVector InputIntervals,
                             ibex::Array<const ibex::ExprSymbol> Variables,
                             ibex::Function *Function): _inputIntervals(InputIntervals),
                                                        _variables(Variables),
                                                        _function(Function) {

}

void IBEXInterface::setInputIntervals(ibex::IntervalVector &InputIntervals) {
  _inputIntervals = InputIntervals;
}

void IBEXInterface::setInputIntervals(std::map<string, FreeVariable *> inputs) {
  double x[inputs.size()][2];
  int i = 0;
  for (auto &input : inputs) {
    x[i][0] = input.second->var->lb();
    x[i][1] = input.second->var->ub();
    i++;
  }
  _inputIntervals = ibex::IntervalVector(inputs.size(), x);
}

void IBEXInterface::setInputIntervals(double x[][2]) {
  _inputIntervals = ibex::IntervalVector(2, x);
}

void IBEXInterface::setVariables(std::map<string, FreeVariable *> inputs,
                                 std::map<string, Node *> table) {
  for (auto &input : inputs) {
    _variables.add(*(ibex::ExprSymbol *) table[input.first]->getExprNode());
  }
}

void IBEXInterface::setFunction(ibex::Function *Function) {
  _function = Function;
}

void IBEXInterface::setFunction(ibex::ExprNode *Expression) {
  _function = new ibex::Function(_variables, *Expression);
}

void IBEXInterface::clearFunction() {
  delete _function;
}

ibex::IntervalVector IBEXInterface::eval() {
  return _function->eval(_inputIntervals);
}