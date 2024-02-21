//
// Created by tanmay on 2/21/24.
//

#ifndef CIRE_RESULTS_H
#define CIRE_RESULTS_H

#include <string>

class Results {
public:
    // The name of the output file
    std::string file;

    Results();
    ~Results();

    void setFile(std::string file);

};


#endif //CIRE_RESULTS_H
