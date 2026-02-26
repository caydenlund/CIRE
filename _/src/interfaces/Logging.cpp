#include "cire/interfaces/Logging.h"

#include <iostream>

// NOLINTNEXTLINE(*-non-const-global-variables)
std::unique_ptr<Logging> logging;

Logging::Logging(std::ostream& out, LogLevel level) : _out(out), level(level) {}

void Logging::debug(const std::string& msg) {
    if (level <= LogLevel::DEBUG) _out << "[DEBUG]  " << msg << "\n";
}

void Logging::info(const std::string& msg) {
    if (level <= LogLevel::INFO) _out << "[INFO]  " << msg << "\n";
}

void Logging::warn(const std::string& msg) {
    if (level <= LogLevel::WARN) _out << "[WARN]  " << msg << "\n";
}

void Logging::error(const std::string& msg) {
    if (level <= LogLevel::ERROR) _out << "[ERROR]  " << msg << "\n";
}

void Logging::critical(const std::string& msg) {
    _out << "[CRITICAL]  " << msg << "\n";
    exit(1);  // NOLINT
}
