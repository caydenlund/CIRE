#ifndef CIRE_IBEXINTERFACE_H
#define CIRE_IBEXINTERFACE_H

#include "ibex.h"
#include "../include/Node.h"
#include <map>

class IBEXInterface {
  ibex::IntervalVector _inputIntervals;
  ibex::Array<const ibex::ExprSymbol> *_variables;
  ibex::Function *_function;

  public:
  IBEXInterface() = default;
  IBEXInterface(ibex::IntervalVector InputIntervals,
                ibex::Array<const ibex::ExprSymbol> Variables,
                ibex::Function *Function);


  void setInputIntervals(ibex::IntervalVector &InputIntervals);
  void setInputIntervals(std::map<string, FreeVariable *> inputs);
  void setInputIntervals(double x[][2]);

  void setVariables(std::map<string, FreeVariable *> inputs,
                    std::map<string, Node *> table);

  void setFunction(ibex::Function *Function);
  void setFunction(ibex::ExprNode *Expression);
  void clearFunction();

  ibex::IntervalVector eval();
  ibex::IntervalVector FindMin(ibex::ExprNode *Expression);
  ibex::IntervalVector FindMax(ibex::ExprNode *Expression);
};

#endif //CIRE_IBEXINTERFACE_H
