#ifndef CIRE_IBEXINTERFACE_H
#define CIRE_IBEXINTERFACE_H

#include "ibex.h"
#include "../include/Node.h"
#include <map>

class OptResult {
public:
    ibex::Interval result;
    ibex::IntervalVector optimumPoint;
    double optimizationTime;
};

class IBEXInterface {
  ibex::IntervalVector _inputIntervals;
  ibex::Array<const ibex::ExprSymbol> *_variables;
  ibex::Function *_function;
  ibex::System *_system;

  public:
  unsigned int debugLevel = 0;
  unsigned int optimizerTimeOut = 0;
  OptResult optResult;

  IBEXInterface() = default;
  IBEXInterface(unsigned int debugLevel,
                ibex::IntervalVector InputIntervals,
                ibex::Array<const ibex::ExprSymbol> Variables,
                ibex::Function *Function,
                ibex::System *System);


  void setInputIntervals(ibex::IntervalVector &InputIntervals);
  void setInputIntervals(std::map<string, FreeVariable *> inputs);
  void setInputIntervals(double x[][2]);

  void setVariables(std::map<string, FreeVariable *> inputs,
                    std::map<string, Node *> table);

  void setFunction(ibex::Function *Function);
  void setFunction(ibex::ExprNode *Expression);
  void setSystem(ibex::SystemFactory *Factory);

  ibex::Function* getFunction();
  ibex::System* getSystem();


  // IBEX Operations
  ibex::IntervalVector eval();
  OptResult FindMin(ibex::ExprNode *Expression);
  OptResult FindMax(ibex::ExprNode *Expression);

  // File IO
  void dumpIbexSystemToFile(std::string filename, ibex::System &System);
  void dumpIbexFunctionToFile(std::string filename, ibex::ExprNode *Expression);
  void dumpIbexExpressionToFile(std::string filename, ibex::ExprNode *Expression);
  ibex::Function parseIbexFunctionFromFile(const char *filename);
  string dumpFunction(ibex::ExprNode *Expression);
};

#endif //CIRE_IBEXINTERFACE_H
