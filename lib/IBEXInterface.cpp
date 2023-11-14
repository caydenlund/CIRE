#include "../include/IBEXInterface.h"

IBEXInterface::IBEXInterface(ibex::IntervalVector InputIntervals,
                             ibex::Array<const ibex::ExprSymbol> Variables,
                             ibex::Function *Function): InputIntervals(InputIntervals),
                                                        Variables(Variables),
                                                        Function(Function) {

}

ibex::IntervalVector IBEXInterface::eval() {
  return Function->eval(InputIntervals);
}