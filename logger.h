#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <chrono>
#include <mutex>
#include <iomanip>

using namespace std;

enum LogType {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class Logger {
public:
    static Logger &getInstance();

    template<typename T> void debug(const T &message, int rank=-1);

    template<typename T> void info(const T &message, int rank=-1);

    template<typename T> void warning(const T &message, int rank=-1);

    template<typename T> void error(const T &message, int rank=-1);

private:
    Logger();

    ~Logger();

    template<typename T> void log(LogType level, const T &message, int rank);

    static string getTimestamp();

    static string getLevelString(LogType level);

    ofstream file_;
    mutex mutex_;
    bool console_output_;
    static Logger *instance_;
    chrono::steady_clock::time_point start_time_;
};

template<typename T> void Logger::debug(const T &message, int rank) {
    log(DEBUG, message, rank);
}

template<typename T> void Logger::info(const T &message, int rank) {
    log(INFO, message, rank);
}

template<typename T> void Logger::warning(const T &message, int rank) {
    log(WARNING, message, rank);
}

template<typename T> void Logger::error(const T &message, int rank) {
    log(ERROR, message, rank);
}

template<typename T> void Logger::log(const LogType level, const T &message, const int rank) {
    lock_guard lock(mutex_);
    stringstream ss;
    ss << "[" << getTimestamp() << "] "
            << "[" << getLevelString(level) << "] ";

    if (rank >= 0) {
        ss << "[Node " << rank << "] ";
    }

    ss << message;

    if (console_output_) {
        cout << ss.str() << '\n';
    }

    if (file_.is_open()) {
        file_ << ss.str() << '\n';
    }
}

#endif //LOGGER_H
