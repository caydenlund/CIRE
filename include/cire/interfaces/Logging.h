#ifndef CIRE_LOGGING_H
#define CIRE_LOGGING_H

#include <cstdint>
#include <memory>
#include <ostream>
#include <sstream>
#include <string>

enum class LogLevel : std::uint8_t {
    DEBUG,
    INFO,
    WARN,
    ERROR,
    CRITICAL,
};

class Logging {
    std::ostream& _out;

    template<typename... ArgsType>
    std::string _formatArgs(ArgsType... args) {
        std::ostringstream oss;
        (oss << ... << args);
        return oss.str();
    }

public:
    LogLevel level;

    Logging(std::ostream& out, LogLevel level = LogLevel::WARN);

    void debug(const std::string& msg);
    void info(const std::string& msg);
    void warn(const std::string& msg);
    void error(const std::string& msg);
    void critical(const std::string& msg);

    template<typename... ArgsType>
    void debug(ArgsType&&... args) {
        if (level <= LogLevel::DEBUG) _out << "[DEBUG]  " << _formatArgs(std::forward<ArgsType>(args)...) << "\n";
    }

    template<typename... ArgsType>
    void info(ArgsType&&... args) {
        if (level <= LogLevel::INFO) _out << "[INFO]  " << _formatArgs(std::forward<ArgsType>(args)...) << "\n";
    }

    template<typename... ArgsType>
    void warn(ArgsType&&... args) {
        if (level <= LogLevel::WARN) _out << "[WARN]  " << _formatArgs(std::forward<ArgsType>(args)...) << "\n";
    }

    template<typename... ArgsType>
    void error(ArgsType&&... args) {
        if (level <= LogLevel::ERROR) _out << "[ERROR]  " << _formatArgs(std::forward<ArgsType>(args)...) << "\n";
    }

    template<typename... ArgsType>
    void critical(ArgsType&&... args) {
        _out << "[CRITICAL]  " << _formatArgs(std::forward<ArgsType>(args)...) << "\n";
        exit(1);  // NOLINT
    }
};

// NOLINTNEXTLINE(*-avoid-non-const-global-variables)
extern std::unique_ptr<Logging> logging;

#endif
