#include "../include/Logging.h"
#include <iostream>

using namespace std;

void Logging::setFile(std::string _file) {
  file = _file;
}

bool Logging::openFile() {
  logFile.open(file);
  if (logFile.is_open()) {
    return true;
  } else {
    return false;
  }
}

bool Logging::closeFile() {
  logFile.close();
  if (logFile.is_open()) {
    return false;
  } else {
    return true;
  }
}

void Logging::log(const std::string& message) {
  logFile << message << endl;
}
