#include "cire/core/Results.h"

#include "cire/core/Graph.hpp"
#include "cire/core/Node.hpp"
#include "cire/interfaces/Logging.h"

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <utility>

Results::Results() : file("results.json") {}

Results::Results(std::string _file) : file(std::move(_file)) {}

Results::~Results() = default;

void Results::setFile(std::string _file) { file = std::move(_file); }

void Results::setStdoutOutput(bool enable) { stdoutOutput = enable; }

void Results::setShowAllInstructions(bool enable) { showAllInstructions = enable; }

bool Results::writeResults(std::vector<std::string> outputs, unsigned int numOperatorsOutput, unsigned int heightDAG,
                           std::map<unsigned int, std::map<std::string, unsigned int>> abstractionMetrics,
                           const std::string& input_file, const std::map<ir::Node*, ErrorAnalysisResult>& results,
                           const std::map<std::string, std::chrono::duration<double>>& time_map,
                           const std::map<ir::Node*, std::vector<InstructionErrorInfo>>& instructionErrors) {
    if (logging) logging->debug("Writing results to ", file, " ...");

    try {
        if (std::filesystem::exists(file)) {
            std::ifstream in(file);
            in >> jsonObject;
        }
        std::filesystem::path file_path = input_file;
        std::string file_stem = file_path.stem().string();
        unsigned k = 0;
        for (auto const& [abs_count, metrics] : abstractionMetrics) {
            jsonObject[file_stem]["Abstraction Metrics"][abs_count]["Window"] = {metrics.at("bound_min"),
                                                                                 metrics.at("bound_max")};
            jsonObject[file_stem]["Abstraction Metrics"][abs_count]["Abstraction Depth"] = metrics.at(
                    "abstraction_depth");
            jsonObject[file_stem]["Abstraction Metrics"][abs_count]["#Candidates"] = metrics.at("num_candidate_nodes");
            jsonObject[file_stem]["Abstraction Metrics"][abs_count]["#Op_Threshold"] = metrics.at(
                    "max_operators_count");
            jsonObject[file_stem]["Abstraction Metrics"][abs_count]["Highest Depth"] = metrics.at("max_depth");
        }
        for (auto const& [node, result] : results) {
            jsonObject[file_stem]["Results"][outputs[k]]["Output"] = {result.outputExtrema.lb(),
                                                                      result.outputExtrema.ub()};
            jsonObject[file_stem]["Results"][outputs[k]]["Error"] = {result.errorExtrema.lb(),
                                                                     result.errorExtrema.ub()};
            jsonObject[file_stem]["Results"]["NumOperators"] = numOperatorsOutput;
            jsonObject[file_stem]["Results"]["Height"] = heightDAG;
            jsonObject[file_stem]["Results"]["Optimization Time"] = result.totalOptimizationTime;
            jsonObject[file_stem]["Results"]["Number of Optimizer Calls"] = result.numOptimizationCalls;
            std::vector<std::pair<double, double>> optPoint;
            optPoint.reserve(result.optPoint.size());
            for (int i = 0; i < result.optPoint.size(); i++) {
                optPoint.emplace_back(result.optPoint[i].lb(), result.optPoint[i].ub());
            }
            jsonObject[file_stem]["Results"][outputs[k]]["Optima"] = optPoint;

            if (instructionErrors.find(node) != instructionErrors.end()) {
                const auto& nodeInstrErrors = instructionErrors.at(node);
                nlohmann::json instrErrorArray = nlohmann::json::array();
                for (const auto& instrError : nodeInstrErrors) {
                    // Skip instructions with zero error contribution unless showAllInstructions is true
                    if (!showAllInstructions && instrError.errorContribution == 0.0) continue;

                    nlohmann::json instrJson;
                    instrJson["instruction_name"] = instrError.instructionName;
                    instrJson["instruction_type"] = instrError.instructionType;
                    instrJson["error_contribution"] = instrError.errorContribution;
                    instrJson["error_bounds"] = {instrError.errorBounds.lb(), instrError.errorBounds.ub()};
                    instrJson["instruction_index"] = instrError.instructionIndex;
                    instrJson["node_id"] = instrError.nodeId;
                    instrJson["percentage_contribution"] = instrError.percentageContribution;
                    instrJson["cumulative_error"] = instrError.cumulativeError;

                    // Add source location if available
                    if (instrError.sourceLocation.isValid()) {
                        instrJson["source_location"] = {{"file", instrError.sourceLocation.filename},
                                                        {"line", instrError.sourceLocation.line},
                                                        {"column", instrError.sourceLocation.column}};
                    }

                    // Add IR representation if available
                    if (!instrError.irRepresentation.empty()) {
                        instrJson["ir_representation"] = instrError.irRepresentation;
                    }

                    instrErrorArray.push_back(instrJson);
                }
                jsonObject[file_stem]["Results"][outputs[k]]["InstructionErrors"] = instrErrorArray;
            }

            k++;
        }
        // Parsing Time + Error Analysis Time = Total Time
        jsonObject[file_stem]["Parsing Time"] = time_map.at("Parsing").count();
        jsonObject[file_stem]["Error Analysis Time"] = time_map.at("Error_Analysis").count();
        jsonObject[file_stem]["Total Time"] = time_map.at("Total").count();
    } catch (std::exception& e) {
        if (logging) logging->error("Failed to write results to JSON file.");
        if (logging) logging->error(e.what());
        return false;
    }

    if (stdoutOutput) {
        // Print compact single-line format for compiler explorer integration
        for (auto const& [node, result] : results) {
            // Summary metrics - one line each
            std::cout << "Output: [" << result.outputExtrema.lb() << ", " << result.outputExtrema.ub() << "]\n";
            std::cout << "Error: [" << result.errorExtrema.lb() << ", " << result.errorExtrema.ub() << "]\n";
            std::cout << "NumOperators: " << numOperatorsOutput << '\n';
            std::cout << "Height: " << heightDAG << '\n';
            std::cout << "Optimization_Time: " << result.totalOptimizationTime << '\n';
            std::cout << "Optimizer_Calls: " << result.numOptimizationCalls << '\n';
            std::cout << "Parsing_Time: " << time_map.at("Parsing").count() << '\n';
            std::cout << "Error_Analysis_Time: " << time_map.at("Error_Analysis").count() << '\n';
            std::cout << "Total_Time: " << time_map.at("Total").count() << '\n';

            // Instruction errors in Compiler Explorer annotation format
            if (instructionErrors.find(node) != instructionErrors.end()) {
                const auto& nodeInstrErrors = instructionErrors.at(node);
                for (const auto& instrError : nodeInstrErrors) {
                    // Skip instructions with zero error contribution unless showAllInstructions is true
                    if (!showAllInstructions && instrError.errorContribution == 0.0) continue;

                    // Use Compiler Explorer format: <file>:<line>:<col>: <severity>: <message>
                    if (instrError.sourceLocation.isValid()) {
                        std::cout << instrError.sourceLocation.filename << ":" << instrError.sourceLocation.line << ":"
                                  << instrError.sourceLocation.column << ": note: " << instrError.instructionName
                                  << " (" << instrError.instructionType << ") - "
                                  << "error contribution: " << std::scientific << std::setprecision(3)
                                  << instrError.errorContribution << " (" << std::fixed << std::setprecision(1)
                                  << instrError.percentageContribution << "%)\n";
                    } else {
                        // Fallback for instructions without source location
                        std::cout << instrError.instructionName << " (" << instrError.instructionType << "): "
                                  << "error contribution: " << std::scientific << std::setprecision(3)
                                  << instrError.errorContribution << " (" << std::fixed << std::setprecision(1)
                                  << instrError.percentageContribution << "%)";
                        // Include IR representation if available for easier correlation
                        if (!instrError.irRepresentation.empty()) std::cout << " | " << instrError.irRepresentation;
                        std::cout << '\n';
                    }
                }
            }

            break;  // Only output first result for stdout
        }
    } else {
        std::ofstream out(file);
        out << std::setw(4) << jsonObject;

        if (logging) logging->debug("Results written to ", file, "!");
    }

    return true;
}

// The following method is only for printing to a CSV file, so we keep the data to at most 2
// dimensions
bool Results::writeResultsForCSV(std::vector<std::string> outputs, unsigned int numOperatorsOutput,
                                 unsigned int heightDAG,
                                 std::map<unsigned int, std::map<std::string, unsigned int>> abstractionMetrics,
                                 const string& input_file, const std::map<ir::Node*, ErrorAnalysisResult>& results,
                                 const std::map<std::string, std::chrono::duration<double>>& time_map,
                                 const std::map<ir::Node*, std::vector<InstructionErrorInfo>>& instructionErrors) {
    if (logging) logging->debug("Writing results to ", file, " ...");

    try {
        if (std::filesystem::exists(file)) {
            std::ifstream in(file);
            in >> jsonObject;
        }
        std::filesystem::path file_path = input_file;
        std::string file_stem = file_path.stem().string();
        for (auto const& [abs_count, metrics] : abstractionMetrics) {
            jsonObject[file_stem]["Window"] = {metrics.at("bound_min"), metrics.at("bound_max")};
            jsonObject[file_stem]["Abstraction Depth"] = metrics.at("abstraction_depth");
            jsonObject[file_stem]["#Candidates"] = metrics.at("num_candidate_nodes");
            jsonObject[file_stem]["#Op_Threshold"] = metrics.at("max_operators_count");
            jsonObject[file_stem]["Highest Depth"] = metrics.at("max_depth");
            break;
        }
        for (auto const& [node, result] : results) {
            jsonObject[file_stem]["Output"] = {result.outputExtrema.lb(), result.outputExtrema.ub()};
            jsonObject[file_stem]["Error"] = {result.errorExtrema.lb(), result.errorExtrema.ub()};
            jsonObject[file_stem]["NumOperators"] = numOperatorsOutput;
            jsonObject[file_stem]["Height"] = heightDAG;
            jsonObject[file_stem]["Optimization Time"] = result.totalOptimizationTime;
            jsonObject[file_stem]["Number of Optimizer Calls"] = result.numOptimizationCalls;
            std::vector<std::pair<double, double>> optPoint;
            for (int i = 0; i < result.optPoint.size(); i++) {
                optPoint.emplace_back(result.optPoint[i].lb(), result.optPoint[i].ub());
            }
            // Fields written to JSON file in lexigraphical order so name the fields accordingly to
            // maintain order of importance. eg: Fields with varied number of values should be
            // placed last in a record.
            jsonObject[file_stem]["zOptima"] = optPoint;
            break;
        }
        // Parsing Time + Error Analysis Time = Total Time
        jsonObject[file_stem]["Parsing Time"] = time_map.at("Parsing").count();
        jsonObject[file_stem]["Error Analysis Time"] = time_map.at("Error_Analysis").count();
        jsonObject[file_stem]["Total Time"] = time_map.at("Total").count();
    } catch (std::exception& e) {
        if (logging) logging->error("Failed to write results to JSON file.");
        if (logging) logging->error(e.what());
        return false;
    }

    if (stdoutOutput) {
        // Print compact single-line format for compiler explorer integration
        for (auto const& [node, result] : results) {
            // Summary metrics - one line each
            std::cout << "Output: [" << result.outputExtrema.lb() << ", " << result.outputExtrema.ub() << "]\n";
            std::cout << "Error: [" << result.errorExtrema.lb() << ", " << result.errorExtrema.ub() << "]\n";
            std::cout << "NumOperators: " << numOperatorsOutput << '\n';
            std::cout << "Height: " << heightDAG << '\n';
            std::cout << "Optimization_Time: " << result.totalOptimizationTime << '\n';
            std::cout << "Optimizer_Calls: " << result.numOptimizationCalls << '\n';
            std::cout << "Parsing_Time: " << time_map.at("Parsing").count() << '\n';
            std::cout << "Error_Analysis_Time: " << time_map.at("Error_Analysis").count() << '\n';
            std::cout << "Total_Time: " << time_map.at("Total").count() << '\n';

            // Instruction errors in Compiler Explorer annotation format
            if (instructionErrors.find(node) != instructionErrors.end()) {
                const auto& nodeInstrErrors = instructionErrors.at(node);
                for (const auto& instrError : nodeInstrErrors) {
                    // Skip instructions with zero error contribution unless showAllInstructions is true
                    if (!showAllInstructions && instrError.errorContribution == 0.0) continue;

                    // Use Compiler Explorer format: <file>:<line>:<col>: <severity>: <message>
                    if (instrError.sourceLocation.isValid()) {
                        std::cout << instrError.sourceLocation.filename << ":" << instrError.sourceLocation.line << ":"
                                  << instrError.sourceLocation.column << ": note: " << instrError.instructionName
                                  << " (" << instrError.instructionType << ") - "
                                  << "error contribution: " << std::scientific << std::setprecision(3)
                                  << instrError.errorContribution << " (" << std::fixed << std::setprecision(1)
                                  << instrError.percentageContribution << "%)\n";
                    } else {
                        // Fallback for instructions without source location
                        std::cout << instrError.instructionName << " (" << instrError.instructionType << "): "
                                  << "error contribution: " << std::scientific << std::setprecision(3)
                                  << instrError.errorContribution << " (" << std::fixed << std::setprecision(1)
                                  << instrError.percentageContribution << "%)";
                        // Include IR representation if available for easier correlation
                        if (!instrError.irRepresentation.empty()) std::cout << " | " << instrError.irRepresentation;
                        std::cout << '\n';
                    }
                }
            }

            break;  // Only output first result for stdout
        }
    } else {
        std::ofstream out(file);
        out << std::setw(4) << jsonObject;

        if (logging) logging->debug("Results written to ", file, "!");
    }

    return true;
}
