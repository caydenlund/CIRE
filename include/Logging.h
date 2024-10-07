#include <string>
#include <fstream>

class Logging {
    std::string file="default.log";
public:
    std::ofstream logFile;
    void setFile(std::string file);
    bool openFile();
    bool closeFile();
    void log(const std::string& message);
};