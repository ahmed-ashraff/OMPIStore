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
    static Logger &getInstance(int rank = -1);

    void setRank(int rank);

    void setLogFile(const string &filename);

    template<typename T> void debug(const T &message);

    template<typename T> void info(const T &message);

    template<typename T> void warning(const T &message);

    template<typename T> void error(const T &message);

private:
    Logger();

    ~Logger();

    template<typename T> void log(LogType level, const T &message);

    string getTimestamp();

    string getLevelString(LogType level);

    int rank_;
    ofstream file_;
    mutex mutex_;
    bool console_output_;
    static Logger *instance_;
    chrono::steady_clock::time_point start_time_;
};

template<typename T> void Logger::debug(const T &message) {
    log(DEBUG, message);
}

template<typename T> void Logger::info(const T &message) {
    log(INFO, message);
}

template<typename T> void Logger::warning(const T &message) {
    log(WARNING, message);
}

template<typename T> void Logger::error(const T &message) {
    log(ERROR, message);
}

template<typename T> void Logger::log(const LogType level, const T &message) {
    lock_guard lock(mutex_);
    stringstream ss;
    ss << "[" << getTimestamp() << "] "
            << "[" << getLevelString(level) << "] ";

    if (rank_ >= 0) {
        ss << "[Node " << rank_ << "] ";
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
