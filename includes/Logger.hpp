#ifndef LOGGER_HPP
#define LOGGER_HPP


#include <iostream>
#include <fstream>
#include <string>
#include <ctime>

class Logger {
private:
    static Logger* instance;
    std::ofstream logFile;
    std::streambuf* oldCoutStreamBuf;
    std::streambuf* oldCerrStreamBuf;
    std::string filename;
    long maxSize;
    int logRotationCount;

    Logger(const std::string& filename, long maxLogSize = 10 * 1024 * 1024); // default max size: 10MB
    void rotateLogs();
    std::string currentTimestamp();

public:
    enum LogLevel {
        DEBUG,
        INFO,
        WARN,
        ERROR
    };

    static Logger* getInstance(const std::string& filename = "logfile.txt", long maxLogSize = 10 * 1024 * 1024);
    void log(LogLevel level, const std::string& message, const std::string& file, int line);
    void captureStdout();
    void releaseStdout();
    void captureStderr();
    void releaseStderr();
    ~Logger();
};

#endif