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

void Logger::setLogFile(const string& filename) {
    if (file_.is_open()) {
        file_.close();
    }
    file_.open(filename, ios::app);
}

string Logger::getTimestamp() {

    // Also get wall clock time for human readability
    const auto wall_time = chrono::system_clock::now();
    const auto wall_time_t = chrono::system_clock::to_time_t(wall_time);

    stringstream ss;
    // Format: [Wall Clock Time] (Monotonic Time)
    ss << put_time(localtime(&wall_time_t), "%Y-%m-%d %H:%M:%S");

    return ss.str();
}

string Logger::getLevelString(const LogType level) {
    switch (level) {
        case DEBUG:   return "DEBUG";
        case INFO:    return "INFO";
        case WARNING: return "WARNING";
        case ERROR:   return "ERROR";
        default:      return "UNKNOWN";
    }
}