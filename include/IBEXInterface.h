#ifndef CIRE_IBEXINTERFACE_H
#define CIRE_IBEXINTERFACE_H

#include "ibex.h"

class IBEXInterface {
  ibex::IntervalVector InputIntervals;
  ibex::Array<const ibex::ExprSymbol> Variables;
  ibex::Function *Function;

  public:
  IBEXInterface(ibex::IntervalVector InputIntervals,
                ibex::Array<const ibex::ExprSymbol> Variables,
                ibex::Function *Function);
  ibex::IntervalVector eval();
};

#endif //CIRE_IBEXINTERFACE_H
