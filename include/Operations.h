#include<map>
#include<math.h>

using namespace std;

#ifndef CIRE_OPERATIONS_H
#define CIRE_OPERATIONS_H

enum RoundingType {
  CONST,
  INT,
  FL32,
  FL64,
};

// The amount of rounding to be applied
map<RoundingType, double> RoundingAmount = {{
  {CONST, 0.0},
  {INT, 0.0},
  {FL32, pow(2, -24+53)},
  {FL64, 1.0},
}};

#endif //CIRE_OPERATIONS_H
