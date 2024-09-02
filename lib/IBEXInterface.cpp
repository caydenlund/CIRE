#include "soplex.h"
#include "../include/IBEXInterface.h"

IBEXInterface::IBEXInterface(ibex::IntervalVector InputIntervals,
                             ibex::Array<const ibex::ExprSymbol> Variables,
                             ibex::Function *Function,
                             ibex::System *System): _inputIntervals(InputIntervals),
                                                        _variables(&Variables),
                                                        _function(Function),
                                                        _system(System) {

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
  delete _variables;
  _variables = new ibex::Array<const ibex::ExprSymbol>;
  for (auto &input : inputs) {
    _variables->add(*(ibex::ExprSymbol *) table[input.first]->getExprNode());
  }
}

void IBEXInterface::setFunction(ibex::Function *Function) {
  _function = Function;
}

void IBEXInterface::setSystem(ibex::SystemFactory *Factory) {
  _system = new ibex::System(*Factory);
}

void IBEXInterface::setFunction(ibex::ExprNode *Expression) {
  delete _function;
  _function = new ibex::Function(*_variables, *Expression);
}

ibex::Function *IBEXInterface::getFunction() {
  return _function;
}

ibex::System *IBEXInterface::getSystem() {
  return _system;
}

ibex::IntervalVector IBEXInterface::eval() {
  return _function->eval(_inputIntervals);
}

ibex::Interval IBEXInterface::FindMin(ibex::ExprNode *Expression) {
  ibex::SystemFactory factory;
  for (auto &var : *_variables) {
    factory.add_var(var);
  }
  factory.add_goal(*Expression);
  setSystem(&factory);
  ibex::DefaultOptimizer opt(*_system);
  try {
    opt.optimize(_inputIntervals);
  } catch (soplex::SPxInternalCodeException &e) {
    std::cerr << "Report to IBEX developers: " << e.what() << std::endl;
    std::cerr << "Rerun till it works" << std::endl;
  }
//  std::cout << "temp: ";
//  opt.report();
  return {opt.get_uplo(), opt.get_loup()};
}

ibex::Interval IBEXInterface::FindMax(ibex::ExprNode *Expression) {
  ibex::SystemFactory factory;
  for (auto &var : *_variables) {
    factory.add_var(var);
  }
  factory.add_goal(-*Expression);
  setSystem(&factory);
  ibex::DefaultOptimizer opt(*_system);
  try {
    opt.optimize(_inputIntervals);
  } catch (soplex::SPxInternalCodeException &e) {
    std::cerr << "Report to IBEX developers: " << e.what() << std::endl;
    std::cerr << "Rerun till it works" << std::endl;
  }
//  std::cout << "temp: ";
//  opt.report();
  return {opt.get_uplo(), opt.get_loup()};
}

void IBEXInterface::dumpIbexFunctionToFile(std::string filename, ibex::ExprNode *Expression) {
  setFunction(Expression);
  std::ofstream file;
  file.open(filename);
  file << _function->minibex() << std::endl;
//  *_function.
  file.close();
}

void IBEXInterface::dumpIbexExpressionToFile(std::string filename, ibex::ExprNode *Expression) {
  std::ofstream file;
  file.open(filename);
  file << *Expression << std::endl;
  file.close();
}

ibex::Function IBEXInterface::parseIbexFunctionFromFile(const char *filename) {
  return ibex::Function(filename);
}
