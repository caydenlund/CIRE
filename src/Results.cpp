#include "Results.h"

Results::Results() {
  file = "results.txt";
}

Results::~Results() {

}

void Results::setFile(std::string file) {
  this->file = file;
}

