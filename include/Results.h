#ifndef CIRE_RESULTS_H
#define CIRE_RESULTS_H

#include <string>
#include <Node.h>
#include <nlohmann/json.hpp>
#include "Graph.h"

class Results {
public:
    // The name of the output file
    std::string file;
    nlohmann::json json_object;
    unsigned int debugLevel = 0;
    Results();
    explicit Results(std::string file);
    ~Results();

    void setFile(std::string file);

    bool writeResults(std::vector<std::string> outputs,
                      unsigned int numOperatorsOutput,
                      unsigned int heightDAG,
                      std::map<unsigned int, std::map<std::string, unsigned int>> abstractionMetrics,
                      const std::string& input_file,
                      const std::map<Node *, ErrorAnalysisResult>& results,
                      const std::map<std::string, std::chrono::duration<double>>& time_map);

    bool writeResultsForCSV(std::vector<std::string> outputs,
                            unsigned int numOperatorsOutput,
                            unsigned int heightDAG,
                            std::map<unsigned int, std::map<std::string, unsigned int>> abstractionMetrics,
                            const std::string& input_file,
                            const std::map<Node *, ErrorAnalysisResult>& results,
                            const std::map<std::string, std::chrono::duration<double>>& time_map);
};


#endif //CIRE_RESULTS_H
