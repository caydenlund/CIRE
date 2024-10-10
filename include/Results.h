#ifndef CIRE_RESULTS_H
#define CIRE_RESULTS_H

#include <string>
#include <Node.h>
#include <nlohmann/json.hpp>

class Results {
public:
    // The name of the output file
    std::string file;
    nlohmann::json json_object;
    unsigned int debugLevel = 0;
    Results();
    ~Results();

    void setFile(std::string file);

    bool writeResults(std::vector<std::string> outputs,
                      std::string input_file,
                      const std::map<Node *, std::vector<ibex::Interval>>& results,
                      const std::map<std::string, std::chrono::duration<double>>& time_map);
};


#endif //CIRE_RESULTS_H
