#ifndef CIRE_CIRE_H
#define CIRE_CIRE_H

#include "cire/core/Graph.h"
#include "cire/core/Results.h"

#include <map>
#include <string>

// Conducting the whole scanning and parsing of Calc++.
class Cire {
public:
    Graph* graph;
    Results* results;
    // The name of the file being parsed.
    std::string file;
    bool abstraction = false;
    std::pair<unsigned int, unsigned int> abstractionWindow = std::make_pair(10, 40);
    // Map of the time taken by each phase of the program.
    std::map<std::string, std::chrono::duration<double>> timeMap;

    explicit Cire(std::string resultFile = "results.json");
    ~Cire();

    void setFile(std::string file);
    void setAbstraction(bool value);
    void setAbstractionWindow(std::pair<unsigned int, unsigned int> window);
    void setMinDepth(unsigned int depth);
    void setMaxDepth(unsigned int depth);
    void setCollectErrorComponentData(bool value) const;
    std::map<Node*, ErrorAnalysisResult> performErrorAnalysis() const;
};

#endif  // CIRE_CIRE_H
