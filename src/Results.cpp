#include <iostream>
#include <utility>
#include <filesystem>
#include "Results.h"

Results::Results() {
  file = "results.txt";
  debugLevel = 0;
}

Results::~Results() = default;

void Results::setFile(std::string _file) {
  file = std::move(_file);
}

bool Results::writeResults(std::vector<std::string> outputs,
                           unsigned int numOperatorsOutput,
                           unsigned int heightDAG,
                           std::map<unsigned int, std::map<std::string, unsigned int>> abstractionMetrics,
                           const std::string& input_file,
                           const std::map<Node *, std::vector<ibex::Interval>>& results,
                           const std::map<std::string, std::chrono::duration<double>>& time_map) {
  if(debugLevel > 0) {
    std::cout << "Writing results to " << file << " ..." << std::endl;
  }

  try {
    if(std::filesystem::exists(file)) {
      std::ifstream in(file);
      in >> json_object;
    }
    std::filesystem::path file_path = input_file;
    std::string file_stem = file_path.stem().string();
    unsigned i = 0;
    for (auto const&[abs_count, metrics] : abstractionMetrics) {
      json_object[file_stem]["Abstraction Metrics"][abs_count]["Window"] =
              {metrics.at("bound_min"), metrics.at("bound_max")};
      json_object[file_stem]["Abstraction Metrics"][abs_count]["Abstraction Depth"] = metrics.at("abstraction_depth");
      json_object[file_stem]["Abstraction Metrics"][abs_count]["#Candidates"] = metrics.at("num_candidate_nodes");
      json_object[file_stem]["Abstraction Metrics"][abs_count]["#Op_Threshold"] = metrics.at("max_operators_count");
      json_object[file_stem]["Abstraction Metrics"][abs_count]["Highest Depth"] = metrics.at("max_depth");
    }
    for (auto const&[node, result] : results) {
      json_object[file_stem]["Results"][outputs[i]]["Output"] = {result[0].lb(), result[0].ub()};
      json_object[file_stem]["Results"][outputs[i]]["Error"] = {result[1].lb(), result[1].ub()};
      json_object[file_stem]["Results"]["NumOperators"] = numOperatorsOutput;
      json_object[file_stem]["Results"]["Height"] = heightDAG;
    }
    json_object[file_stem]["Parsing Time"] = time_map.at("Parsing").count();
    json_object[file_stem]["Error Analysis Time"] = time_map.at("Error_Analysis").count();
    json_object[file_stem]["Total Time"] = time_map.at("Total").count();
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