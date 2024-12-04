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
      std::vector<std::pair<double, double>> lbPoint;
      std::vector<std::pair<double, double>> ubPoint;
      for (int i = 0; i < result.lbPoint.size(); i++) {
        lbPoint.emplace_back(result.lbPoint[i].lb(), result.lbPoint[i].ub());
        ubPoint.emplace_back(result.ubPoint[i].lb(), result.ubPoint[i].ub());
      }
      json_object[file_stem]["Results"][outputs[k]]["Lower Bound Optima"] = lbPoint;
      json_object[file_stem]["Results"][outputs[k]]["Upper Bound Optima"] = ubPoint;
      json_object[file_stem]["Results"]["NumOperators"] = numOperatorsOutput;
      json_object[file_stem]["Results"]["Height"] = heightDAG;
      json_object[file_stem]["Results"]["Optimization Time"] = result.totalOptimizationTime;
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
      std::vector<std::pair<double, double>> lbPoint;
      std::vector<std::pair<double, double>> ubPoint;
      for (int i = 0; i < result.lbPoint.size(); i++) {
        lbPoint.emplace_back(result.lbPoint[i].lb(), result.lbPoint[i].ub());
        ubPoint.emplace_back(result.ubPoint[i].lb(), result.ubPoint[i].ub());
      }
      json_object[file_stem]["Lower Bound Optima"] = lbPoint;
      json_object[file_stem]["Upper Bound Optima"] = ubPoint;
      json_object[file_stem]["NumOperators"] = numOperatorsOutput;
      json_object[file_stem]["Height"] = heightDAG;
      json_object[file_stem]["Optimization Time"] = result.totalOptimizationTime;
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
