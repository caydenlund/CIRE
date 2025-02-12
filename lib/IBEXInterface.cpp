#include "soplex.h"
#include "../include/IBEXInterface.h"

IBEXInterface::IBEXInterface(unsigned int debugLevel,
                             ibex::IntervalVector InputIntervals,
                             ibex::Array<const ibex::ExprSymbol> Variables,
                             ibex::Function *Function,
                             ibex::System *System): debugLevel(debugLevel),
                                                    _inputIntervals(InputIntervals),
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

void IBEXInterface::setFunction(ibex::ExprNode *Expression) {
  delete _function;
  _function = new ibex::Function(*_variables, *Expression);
}

void IBEXInterface::setSystem(ibex::SystemFactory *Factory) {
  _system = new ibex::System(*Factory);
}

ibex::Array<const ibex::ExprSymbol> *IBEXInterface::getVariables() {
  return _variables;
}

ibex::IntervalVector IBEXInterface::getInputIntervals() {
  return _inputIntervals;
}

ibex::Function *IBEXInterface::getFunction() {
  return _function;
}

ibex::System *IBEXInterface::getSystem() {
  return _system;
}

ibex::Interval IBEXInterface::eval() {
  return _function->eval(_inputIntervals);
}

ibex::Interval IBEXInterface::eval(ibex::Function &Function) {
  return Function.eval(_inputIntervals);
}

OptResult IBEXInterface::FindMin(ibex::ExprNode &Expression) {
  ibex::SystemFactory factory;
  for (auto &var : *_variables) {
    factory.add_var(var);
  }
  factory.add_goal(Expression);
  setSystem(&factory);
  ibex::DefaultOptimizerConfig optConfig(*_system,
                                          ibex::OptimizerConfig::default_rel_eps_f,
                                          ibex::OptimizerConfig::default_abs_eps_f,
                                          ibex::NormalizedSystem::default_eps_h,
//                                         1e-01,
//                                         1e-01,
//                                         1e-01,
                                         false,
                                         ibex::DefaultOptimizerConfig::default_inHC4,
                                         false,
                                         ibex::DefaultOptimizerConfig::default_random_seed,
                                         ibex::OptimizerConfig::default_eps_x);
  if (optimizerTimeOut > 0) {
    optConfig.set_timeout(optimizerTimeOut);
  }
  ibex::Optimizer opt(optConfig);
  try {
    if(debugLevel > 3) {
      // Remove the last "end" keyword from the file before using with IBEX to avoid syntax errors
      dumpIbexSystemToFile("ibexFunctionMin.txt", *_system);
      std::cout << "Input Intervals: " << _inputIntervals << std::endl;
      std::cout << "Variables: ";
      for (auto &var : *_variables) {
        std:: cout << var << ", ";
      }
      std::cout << std::endl;
    }

    opt.optimize(_inputIntervals);
  } catch (soplex::SPxInternalCodeException &e) {
    std::cerr << "Report to IBEX developers: " << e.what() << std::endl;
    std::cerr << "Rerun till it works" << std::endl;
  }
  if(opt.get_status() == ibex::Optimizer::INFEASIBLE) {
    std::cerr << "Optimizer returned INFEASIBLE. This case is not possible as we do not set an initial bound. "
                 "Read up status description in IBEX documentation and investigate." << std::endl;
    exit(1);
  } else if(opt.get_status() == ibex::Optimizer::NO_FEASIBLE_FOUND) {
    std::cerr << "Optimizer returned NO_FEASIBLE_FOUND. This case is not possible as we do not set an initial bound. "
                 "Read up status description in IBEX documentation and investigate." << std::endl;
    exit(1);
  } else if(opt.get_status() == ibex::Optimizer::UNBOUNDED_OBJ) {
    std::cerr << "Optimizer returned UNBOUNDED. Objective tends to inf or -inf." << std::endl;
    exit(1);
  }  else if(opt.get_status() == ibex::Optimizer::UNREACHED_PREC) {
    std::cerr << "Optimizer returned UNREACHED_PREC. Might need to change objective, constrains or objective precision." << std::endl;
    exit(1);
  } else if(opt.get_status() == ibex::Optimizer::TIME_OUT) {
    std::cerr << "Optimizer TIMEOUT." << std::endl;
    exit(1);
  }
//  opt.report();
  optResult.result = ibex::Interval(opt.get_uplo(), opt.get_loup());
  optResult.optimumPoint = opt.get_loup_point();
  optResult.optimizationTime = opt.get_time();

  return optResult;
}

OptResult IBEXInterface::FindMax(ibex::ExprNode &Expression) {
  ibex::SystemFactory factory;
  for (auto &var : *_variables) {
    factory.add_var(var);
  }
  factory.add_goal(-Expression);
  setSystem(&factory);
  ibex::DefaultOptimizerConfig optConfig(*_system,
                                           ibex::OptimizerConfig::default_rel_eps_f,
                                           ibex::OptimizerConfig::default_abs_eps_f,
                                           ibex::NormalizedSystem::default_eps_h,
//                                         1e-01,
//                                         1e-01,
//                                         1e-01,
                                         false,
                                         ibex::DefaultOptimizerConfig::default_inHC4,
                                         false,
                                         ibex::DefaultOptimizerConfig::default_random_seed,
                                         ibex::OptimizerConfig::default_eps_x);
  if(optimizerTimeOut > 0) {
    optConfig.set_timeout(optimizerTimeOut);
  }
  ibex::Optimizer opt(optConfig);

  try {
    if(debugLevel > 2) {
      // Remove the last "end" keyword from the file before using with IBEX to avoid syntax errors
      dumpIbexSystemToFile("ibexFunctionMax.txt", *_system);
      std::cout << "Input Intervals: " << _inputIntervals << std::endl;
      std::cout << "Variables: ";
      for (auto &var : *_variables) {
        std:: cout << var << ", ";
      }
      std::cout << std::endl;
    }
    opt.optimize(_inputIntervals);
  } catch (soplex::SPxInternalCodeException &e) {
    std::cerr << "Report to IBEX developers: " << e.what() << std::endl;
    std::cerr << "Rerun till it works" << std::endl;
  }

  if(opt.get_status() == ibex::Optimizer::INFEASIBLE) {
    std::cerr << "Optimizer returned INFEASIBLE. This case is not possible as we do not set an initial bound. "
                 "Read up status description in IBEX documentation and investigate." << std::endl;
    exit(1);
  } else if(opt.get_status() == ibex::Optimizer::NO_FEASIBLE_FOUND) {
    std::cerr << "Optimizer returned NO_FEASIBLE_FOUND. This case is not possible as we do not set an initial bound. "
                 "Read up status description in IBEX documentation and investigate." << std::endl;
    exit(1);
  } else if(opt.get_status() == ibex::Optimizer::UNBOUNDED_OBJ) {
    std::cerr << "Optimizer returned UNBOUNDED. Objective tends to inf or -inf." << std::endl;
    exit(1);
  }  else if(opt.get_status() == ibex::Optimizer::UNREACHED_PREC) {
    std::cerr << "Optimizer returned UNREACHED_PREC. Might need to change objective, constrains or objective precision." << std::endl;
    exit(1);
  } else if(opt.get_status() == ibex::Optimizer::TIME_OUT) {
    std::cerr << "Optimizer TIMEOUT." << std::endl;
    exit(1);
  }
//  opt.report();
  optResult.result = ibex::Interval(opt.get_uplo(), opt.get_loup());
  optResult.optimumPoint = opt.get_loup_point();
  optResult.optimizationTime = opt.get_time();

  return optResult;
}

void IBEXInterface::dumpIbexSystemToFile(std::string filename, ibex::System &System) {
  std::ofstream file;
  file.open(filename);
  // Remove the last "end" keyword from the file before using with IBEX to avoid syntax errors
  file << System.minibex();
  file.close();
}

void IBEXInterface::dumpIbexFunctionToFile(std::string filename, ibex::ExprNode *Expression) {
  setFunction(Expression);
  std::ofstream file;
  file.open(filename);
  file << _function->minibex();
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

string IBEXInterface::dumpFunction(ibex::ExprNode *Expression) {
  setFunction(Expression);
  return _function->minibex();
}