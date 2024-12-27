#include "logger.h"

using namespace std;

Logger* Logger::instance_ = nullptr;

Logger::Logger()
    : rank_(-1)
    , console_output_(true)
    , start_time_(std::chrono::steady_clock::now()) {}

Logger::~Logger() {
    if (file_.is_open()) {
        file_.close();
    }
}

Logger& Logger::getInstance(const int rank) {
    if (instance_ == nullptr) {
        instance_ = new Logger();
    }
    if (rank >= 0) {
        instance_->setRank(rank);
    }
    return *instance_;
}

void Logger::setRank(const int rank) {
    rank_ = rank;
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
        case DEBUG:   return "\033[36mDEBUG\033[0m";   // Cyan text for DEBUG
        case INFO:    return "INFO";
        case WARNING: return "\033[33mWARNING\033[0m"; // Yellow text for WARNING
        case ERROR:   return "\033[31mERROR\033[0m";   // Red text for ERROR
        default:      return "UNKNOWN";
    }
}