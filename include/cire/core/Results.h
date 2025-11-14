#ifndef CIRE_RESULTS_H
#define CIRE_RESULTS_H

#include "cire/core/InstructionMetadata.h"
#include "ibex_Interval.h"

#include <nlohmann/json.hpp>
#include <string>
#include <map>
#include <vector>
#include <chrono>

// Forward declarations
class Node;
class ErrorAnalysisResult;

struct InstructionErrorInfo {
    std::string instructionName;
    std::string instructionType;
    double errorContribution;
    ibex::Interval errorBounds;
    int instructionIndex;

    // Enhanced fields for richer reporting
    double percentageContribution = 0.0;  // Percentage of total error
    double cumulativeError = 0.0;          // Cumulative error up to this instruction
    SourceLocation sourceLocation;         // Source code location
    std::string irRepresentation;          // LLVM IR string
    int nodeId = -1;                       // CIRE node ID

    // Sort comparator by error contribution (descending)
    bool operator<(const InstructionErrorInfo& other) const {
        return errorContribution > other.errorContribution;
    }
};

class Results {
public:
    // The name of the output file
    std::string file;
    nlohmann::json jsonObject;
    bool stdoutOutput = false;
    Results();
    explicit Results(std::string file);
    ~Results();

    void setFile(std::string file);
    void setStdoutOutput(bool enable);

    bool writeResults(std::vector<std::string> outputs, unsigned int numOperatorsOutput, unsigned int heightDAG,
                      std::map<unsigned int, std::map<std::string, unsigned int>> abstractionMetrics,
                      const std::string& input_file, const std::map<Node*, ErrorAnalysisResult>& results,
                      const std::map<std::string, std::chrono::duration<double>>& time_map,
                      const std::map<Node*, std::vector<InstructionErrorInfo>>& instructionErrors = {});

    bool writeResultsForCSV(std::vector<std::string> outputs, unsigned int numOperatorsOutput, unsigned int heightDAG,
                            std::map<unsigned int, std::map<std::string, unsigned int>> abstractionMetrics,
                            const std::string& input_file, const std::map<Node*, ErrorAnalysisResult>& results,
                            const std::map<std::string, std::chrono::duration<double>>& time_map,
                            const std::map<Node*, std::vector<InstructionErrorInfo>>& instructionErrors = {});
};


#endif  // CIRE_RESULTS_H
