#include <iostream>
#include <utility>
#include "Results.h"

Results::Results() {
  file = "results.txt";
  debugLevel = 0;
  logLevel = 0;
}

Results::~Results() = default;

void Results::setFile(std::string _file) {
  file = std::move(_file);
}

bool Results::writeResults(std::vector<std::string> outputs,
                           const std::map<Node *, std::vector<ibex::Interval>>& results,
                           const std::map<std::string, std::chrono::duration<double>>& time_map) {
  if(debugLevel > 0) {
    std::cout << "Writing results to " << file << " ..." << std::endl;
  }

  try {
    unsigned i = 0;
    for (auto const&[node, result] : results) {
      json_object["Results"][outputs[i]]["Output"] = {result[0].lb(), result[0].ub()};
      json_object["Results"][outputs[i]]["Error"] = {result[1].lb(), result[1].ub()};
    }
    json_object["Total Time"] = time_map.at("Total").count();
  } catch (std::exception &e) {
    std::cout << "[ERROR]: Failed to write results to JSON file." << std::endl;
    std::cout << e.what() << std::endl;
    return false;
  }
  std::ofstream out(file);
  out << std::setw(4) << json_object;

  if(debugLevel > 0) {
    std::cout << "Results written to " << file << "!" << std::endl;
  }

  return true;
}