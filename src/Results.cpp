#include <iostream>
#include <utility>
#include <filesystem>
#include "Results.h"

Results::Results() {
  file = "results.json";
  debugLevel = 0;
}

Results::Results(std::string _file) {
  file = std::move(_file);
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
                           const std::map<Node *, ErrorAnalysisResult>& results,
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
    unsigned k = 0;
    for (auto const&[abs_count, metrics] : abstractionMetrics) {
      json_object[file_stem]["Abstraction Metrics"][abs_count]["Window"] =
              {metrics.at("bound_min"), metrics.at("bound_max")};
      json_object[file_stem]["Abstraction Metrics"][abs_count]["Abstraction Depth"] = metrics.at("abstraction_depth");
      json_object[file_stem]["Abstraction Metrics"][abs_count]["#Candidates"] = metrics.at("num_candidate_nodes");
      json_object[file_stem]["Abstraction Metrics"][abs_count]["#Op_Threshold"] = metrics.at("max_operators_count");
      json_object[file_stem]["Abstraction Metrics"][abs_count]["Highest Depth"] = metrics.at("max_depth");
    }
    for (auto const&[node, result] : results) {
      json_object[file_stem]["Results"][outputs[k]]["Output"] = {result.outputExtrema.lb(), result.outputExtrema.ub()};
      json_object[file_stem]["Results"][outputs[k]]["Error"] = {result.errorExtrema.lb(), result.errorExtrema.ub()};
      json_object[file_stem]["Results"]["NumOperators"] = numOperatorsOutput;
      json_object[file_stem]["Results"]["Height"] = heightDAG;
      json_object[file_stem]["Results"]["Optimization Time"] = result.totalOptimizationTime;
      json_object[file_stem]["Results"]["Number of Optimizer Calls"] = result.numOptimizationCalls;
      std::vector<std::pair<double, double>> optPoint;
      for (int i = 0; i < result.OptPoint.size(); i++) {
        optPoint.emplace_back(result.OptPoint[i].lb(), result.OptPoint[i].ub());
      }
      json_object[file_stem]["Results"][outputs[k]]["Optima"] = optPoint;
      k++;
    }
    // Parsing Time + Error Analysis Time = Total Time
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

// The following method is only for printing to a CSV file, so we keep the data to at most 2 dimensions
bool
Results::writeResultsForCSV(std::vector<std::string> outputs, unsigned int numOperatorsOutput, unsigned int heightDAG,
                            std::map<unsigned int, std::map<std::string, unsigned int>> abstractionMetrics,
                            const string &input_file, const std::map<Node *, ErrorAnalysisResult> &results,
                            const std::map<std::string, std::chrono::duration<double>> &time_map) {
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
    for (auto const&[abs_count, metrics] : abstractionMetrics) {
      json_object[file_stem]["Window"] =
              {metrics.at("bound_min"), metrics.at("bound_max")};
      json_object[file_stem]["Abstraction Depth"] = metrics.at("abstraction_depth");
      json_object[file_stem]["#Candidates"] = metrics.at("num_candidate_nodes");
      json_object[file_stem]["#Op_Threshold"] = metrics.at("max_operators_count");
      json_object[file_stem]["Highest Depth"] = metrics.at("max_depth");
      break;
    }
    for (auto const&[node, result] : results) {
      json_object[file_stem]["Output"] = {result.outputExtrema.lb(), result.outputExtrema.ub()};
      json_object[file_stem]["Error"] = {result.errorExtrema.lb(), result.errorExtrema.ub()};
      json_object[file_stem]["NumOperators"] = numOperatorsOutput;
      json_object[file_stem]["Height"] = heightDAG;
      json_object[file_stem]["Optimization Time"] = result.totalOptimizationTime;
      json_object[file_stem]["Number of Optimizer Calls"] = result.numOptimizationCalls;
      std::vector<std::pair<double, double>> optPoint;
      for (int i = 0; i < result.OptPoint.size(); i++) {
        optPoint.emplace_back(result.OptPoint[i].lb(), result.OptPoint[i].ub());
      }
      // Fields written to JSON file in lexigraphical order so name the fields accordingly to maintain
      // order of importance.
      // eg: Fields with varied number of values should be placed last in a record.
      json_object[file_stem]["zOptima"] = optPoint;
      break;
    }
    // Parsing Time + Error Analysis Time = Total Time
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
