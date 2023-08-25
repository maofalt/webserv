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

/*
 * DEBUG LOGGING MACROS:
 *
 * These macros provide a mechanism for conditional debug logging based on the defined DEBUG level.
 *
 * 1. DEBUG_LOG:
 *    - Used for general debug messages.
 *    - The message will be logged if the DEBUG preprocessor directive is defined, regardless of its value.
 *
 * 2. DEBUG_LOG_DETAILED:
 *    - Used for detailed debug messages.
 *    - The message will be logged only if the DEBUG preprocessor directive is defined with a value of 2 or greater.
 *
 * HOW IT WORKS:
 * The macros utilize the Logger class to log messages with the specified level (DEBUG or DEBUG_DETAILED), along with the file name and line number.
 * When DEBUG is not defined or does not meet the required level, the macros expand to nothing, effectively removing the log calls from the compiled code.
 * 
 * WHY AT THE END OF THE CODE?
 * Placing these macros at the end ensures that the full Logger class definition and any other necessary dependencies are already defined and included.
 * This ensures smooth compilation and avoids potential issues with undefined symbols or incomplete types.
 *
 * "OVERLOADING":
 * While true macro overloading doesn't exist in C++, the use of different macro names (DEBUG_LOG and DEBUG_LOG_DETAILED) mimics function overloading behavior,
 * allowing developers to choose the appropriate logging level based on their needs.
 */

#ifdef DEBUG
#define DEBUG_LOG(msg) Logger::getInstance()->log(Logger::DEBUG, msg, __FILE__, __LINE__)
#if DEBUG >= 2
#define DEBUG_LOG_DETAILED(msg) Logger::getInstance()->log(Logger::DEBUG_DETAILED, msg, __FILE__, __LINE__)
#else
#define DEBUG_LOG_DETAILED(msg)
#endif
#else
#define DEBUG_LOG(msg) 
#define DEBUG_LOG_DETAILED(msg)
#endif

#endif