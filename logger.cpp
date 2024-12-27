#include "logger.h"

using namespace std;

Logger *Logger::instance_ = nullptr;

Logger::Logger() : console_output_(true), start_time_(chrono::steady_clock::now()) {}

Logger::~Logger() {
    if (file_.is_open()) {
        file_.close();
    }
}

Logger &Logger::getInstance() {
    if (instance_ == nullptr) {
        instance_ = new Logger();
    }
    return *instance_;
}

string Logger::getTimestamp() {
    const auto wall_time = chrono::system_clock::now();
    const auto wall_time_t = chrono::system_clock::to_time_t(wall_time);

    stringstream ss;
    // Format: [Wall Clock Time] (Monotonic Time)
    ss << put_time(localtime(&wall_time_t), "%Y-%m-%d %H:%M:%S");

    return ss.str();
}

string Logger::getLevelString(const LogType level) {
    switch (level) {
        case DEBUG: return "\033[36mDEBUG\033[0m"; // Cyan text for DEBUG
        case INFO: return "INFO";
        case WARNING: return "\033[33mWARNING\033[0m"; // Yellow text for WARNING
        case ERROR: return "\033[31mERROR\033[0m"; // Red text for ERROR
        default: return "UNKNOWN";
    }
}
