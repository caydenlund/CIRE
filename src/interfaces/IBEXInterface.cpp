#include "cire/interfaces/IBEXInterface.h"
#include "cire/interfaces/Logging.h"
#include "ibex_DefaultOptimizerConfig.h"
#include "ibex_Optimizer.h"
#include "ibex_SystemFactory.h"

IBEXInterface::IBEXInterface(ibex::IntervalVector InputIntervals, ibex::Array<const ibex::ExprSymbol> Variables,
                             ibex::Function* Function, ibex::System* System)
    : _inputIntervals(InputIntervals), _variables(&Variables), _function(Function), _system(System) {}

void IBEXInterface::setInputIntervals(ibex::IntervalVector& InputIntervals) { _inputIntervals = InputIntervals; }

void IBEXInterface::setInputIntervals(std::map<string, ir::FreeVariable*> inputs) {
    double x[inputs.size()][2];
    int i = 0;
    for (auto& input : inputs) {
        x[i][0] = input.second->var->lb();
        x[i][1] = input.second->var->ub();
        i++;
    }
    _inputIntervals = ibex::IntervalVector(int(inputs.size()), (double (*)[2])x);
}

void IBEXInterface::setInputIntervals(double x[][2]) { _inputIntervals = ibex::IntervalVector(2, x); }

void IBEXInterface::setVariables(std::map<string, ir::FreeVariable*> inputs, std::map<string, ir::Node*> table) {
    delete _variables;
    _variables = new ibex::Array<const ibex::ExprSymbol>;
    for (auto& input : inputs) _variables->add(*dynamic_cast<ibex::ExprSymbol*>(table[input.first]->getExprNode()));
}

void IBEXInterface::setFunction(ibex::Function* Function) { _function = Function; }

void IBEXInterface::setFunction(ibex::ExprNode* Expression) {
    delete _function;
    _function = new ibex::Function(*_variables, *Expression);
}

void IBEXInterface::setSystem(ibex::SystemFactory* Factory) { _system = new ibex::System(*Factory); }

ibex::Array<const ibex::ExprSymbol>* IBEXInterface::getVariables() { return _variables; }

ibex::IntervalVector IBEXInterface::getInputIntervals() { return _inputIntervals; }

ibex::Function* IBEXInterface::getFunction() { return _function; }

ibex::System* IBEXInterface::getSystem() { return _system; }

ibex::Interval IBEXInterface::eval() { return _function->eval(_inputIntervals); }

ibex::Interval IBEXInterface::eval(ibex::Function& Function) { return Function.eval(_inputIntervals); }

OptResult IBEXInterface::findMin(ibex::ExprNode& Expression) {
    ibex::SystemFactory factory;
    for (const auto& var : *_variables) factory.add_var(var);
    factory.add_goal(Expression);
    setSystem(&factory);
    ibex::DefaultOptimizerConfig optConfig(
            *_system, ibex::OptimizerConfig::default_rel_eps_f, ibex::OptimizerConfig::default_abs_eps_f,
            ibex::NormalizedSystem::default_eps_h,
            //                                         1e-01,
            //                                         1e-01,
            //                                         1e-01,
            false, ibex::DefaultOptimizerConfig::default_inHC4, false,
            ibex::DefaultOptimizerConfig::default_random_seed, ibex::OptimizerConfig::default_eps_x);
    if (optimizerTimeOut > 0) optConfig.set_timeout(optimizerTimeOut);
    ibex::Optimizer opt(optConfig);
    try {
        if (logging && logging->level <= LogLevel::DEBUG) {
            // Remove the last "end" keyword from the file before using with IBEX to avoid syntax
            // errors
            dumpIbexSystemToFile("ibexFunctionMin.txt", *_system);
            std::cout << "Input Intervals: " << _inputIntervals << '\n';
            std::cout << "Variables: ";
            for (const auto& var : *_variables) std::cout << var << ", ";
            std::cout << '\n';
        }

        opt.optimize(_inputIntervals);
    } catch (soplex::SPxInternalCodeException& e) {
        if (logging) {
            logging->error("Report to IBEX developers: ", e.what());
            logging->error("Rerun till it works");
        }
    }
    if (opt.get_status() == ibex::Optimizer::INFEASIBLE) {
        if (logging) {
            logging->critical("Optimizer returned INFEASIBLE. This case is not possible as we do not set an initial "
                              "bound. Read up status description in IBEX documentation and investigate.");
        }
    } else if (opt.get_status() == ibex::Optimizer::NO_FEASIBLE_FOUND) {
        if (logging) {
            logging->critical("Optimizer returned NO_FEASIBLE_FOUND. This case is not possible as we do not set an "
                              "initial bound. Read up status description in IBEX documentation and investigate.");
        }
    } else if (opt.get_status() == ibex::Optimizer::UNBOUNDED_OBJ) {
        if (logging) logging->error("Optimizer returned UNBOUNDED. Objective tends to inf or -inf.");
        optResult.result = ibex::Interval(std::numeric_limits<double>::min(), std::numeric_limits<double>::min());
        optResult.optimumPoint = opt.get_loup_point();
        optResult.optimizationTime = opt.get_time();
    } else if (opt.get_status() == ibex::Optimizer::UNREACHED_PREC) {
        if (logging) {
            logging->critical("Optimizer returned UNREACHED_PREC. Might need to change objective, constrains or "
                              "objective precision.");
        }
    } else if (opt.get_status() == ibex::Optimizer::TIME_OUT) {
        if (logging) logging->error("Optimizer TIMEOUT. Default error.");
        optResult.result = ibex::Interval(std::numeric_limits<double>::min(), std::numeric_limits<double>::min());
        optResult.optimumPoint = opt.get_loup_point();
        optResult.optimizationTime = opt.get_time();
    } else {
        optResult.result = ibex::Interval(opt.get_uplo(), opt.get_loup());
        optResult.optimumPoint = opt.get_loup_point();
        optResult.optimizationTime = opt.get_time();
    }

    //  opt.report();

    return optResult;
}

OptResult IBEXInterface::findMax(ibex::ExprNode& Expression) {
    ibex::SystemFactory factory;
    for (const auto& var : *_variables) factory.add_var(var);
    factory.add_goal(-Expression);
    setSystem(&factory);
    ibex::DefaultOptimizerConfig optConfig(
            *_system, ibex::OptimizerConfig::default_rel_eps_f, ibex::OptimizerConfig::default_abs_eps_f,
            ibex::NormalizedSystem::default_eps_h,
            //                                         1e-01,
            //                                         1e-01,
            //                                         1e-01,
            false, ibex::DefaultOptimizerConfig::default_inHC4, false,
            ibex::DefaultOptimizerConfig::default_random_seed, ibex::OptimizerConfig::default_eps_x);
    if (optimizerTimeOut > 0) optConfig.set_timeout(optimizerTimeOut);
    ibex::Optimizer opt(optConfig);

    try {
        if (logging && logging->level <= LogLevel::DEBUG) {
            // Remove the last "end" keyword from the file before using with IBEX to avoid syntax
            // errors
            dumpIbexSystemToFile("ibexFunctionMax.txt", *_system);
            std::cout << "Input Intervals: " << _inputIntervals << '\n';
            std::cout << "Variables: ";
            for (const auto& var : *_variables) std::cout << var << ", ";
            std::cout << '\n';
        }
        opt.optimize(_inputIntervals);
    } catch (soplex::SPxInternalCodeException& e) {
        if (logging) {
            logging->error("Report to IBEX developers: ", e.what());
            logging->error("Rerun till it works");
        }
    }

    if (opt.get_status() == ibex::Optimizer::INFEASIBLE) {
        if (logging) {
            logging->critical("Optimizer returned INFEASIBLE. This case is not possible as we do not set an initial "
                              "bound. Read up status description in IBEX documentation and investigate.");
        }
    } else if (opt.get_status() == ibex::Optimizer::NO_FEASIBLE_FOUND) {
        if (logging) {
            logging->critical("Optimizer returned NO_FEASIBLE_FOUND. This case is not possible as we do not set an "
                              "initial bound. Read up status description in IBEX documentation and investigate.");
        }
    } else if (opt.get_status() == ibex::Optimizer::UNBOUNDED_OBJ) {
        if (logging) logging->error("Optimizer returned UNBOUNDED. Objective tends to inf or -inf.");
        optResult.result = ibex::Interval(std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
        optResult.optimumPoint = opt.get_loup_point();
        optResult.optimizationTime = opt.get_time();
    } else if (opt.get_status() == ibex::Optimizer::UNREACHED_PREC) {
        if (logging) {
            logging->critical("Optimizer returned UNREACHED_PREC. Might need to change objective, constrains or "
                              "objective precision.");
        }
    } else if (opt.get_status() == ibex::Optimizer::TIME_OUT) {
        if (logging) logging->error("Optimizer TIMEOUT. Default error.");
        optResult.result = ibex::Interval(std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
        optResult.optimumPoint = opt.get_loup_point();
        optResult.optimizationTime = opt.get_time();
    } else {
        optResult.result = ibex::Interval(opt.get_uplo(), opt.get_loup());
        optResult.optimumPoint = opt.get_loup_point();
        optResult.optimizationTime = opt.get_time();
    }

    //  opt.report();


    return optResult;
}

OptResult IBEXInterface::findAbsMax(ibex::ExprNode& Expression) { return findMax((ibex::ExprNode&)abs(Expression)); }

void IBEXInterface::dumpIbexSystemToFile(std::string filename, ibex::System& System) {
    std::ofstream file;
    file.open(filename);
    // Remove the last "end" keyword from the file before using with IBEX to avoid syntax errors
    file << System.minibex();
    file.close();
}

void IBEXInterface::dumpIbexFunctionToFile(std::string filename, ibex::ExprNode* Expression) {
    setFunction(Expression);
    std::ofstream file;
    file.open(filename);
    file << _function->minibex();
    //  *_function.
    file.close();
}

void IBEXInterface::dumpIbexExpressionToFile(std::string filename, ibex::ExprNode* Expression) {
    std::ofstream file;
    file.open(filename);
    file << *Expression << '\n';
    file.close();
}

ibex::Function IBEXInterface::parseIbexFunctionFromFile(const char* filename) { return {filename}; }

string IBEXInterface::dumpFunction(ibex::ExprNode* Expression) {
    setFunction(Expression);
    return _function->minibex();
}
