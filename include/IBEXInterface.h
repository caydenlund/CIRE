#ifndef CIRE_IBEXINTERFACE_H
#define CIRE_IBEXINTERFACE_H

#include "ibex.h"
#include "../include/Node.h"
#include <map>

class IBEXInterface {
  ibex::IntervalVector _inputIntervals;
  ibex::Array<const ibex::ExprSymbol> *_variables;
  ibex::Function *_function;
  ibex::System *_system;

  public:
  IBEXInterface() = default;
  IBEXInterface(ibex::IntervalVector InputIntervals,
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
  ibex::Interval FindMin(ibex::ExprNode *Expression);
  ibex::Interval FindMax(ibex::ExprNode *Expression);

  // File IO
  void dumpIbexFunctionToFile(std::string filename, ibex::ExprNode *Expression);
  void dumpIbexExpressionToFile(std::string filename, ibex::ExprNode *Expression);
  ibex::Function parseIbexFunctionFromFile(const char *filename);
  string dumpFunction(ibex::ExprNode *Expression);
};

#endif //CIRE_IBEXINTERFACE_H
