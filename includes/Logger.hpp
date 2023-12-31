#ifndef LOGGER_HPP
#define LOGGER_HPP


#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <string>
#include <ctime>
#include <cassert>
#include <vector>
#include <sys/stat.h>
#include "TeeBuf.hpp"
#include "Colors.hpp"

#define SEPARATOR_START "╔════════════════════════════════════════════════════════════════════════════════════════════════════╗"
#define SEPARATOR_END   "╚════════════════════════════════════════════════════════════════════════════════════════════════════╝"
#define TIMESTAMP_WIDTH 22
#define LEVEL_WIDTH     8
#define FILE_FUNC_WIDTH 25
#define MESSAGE_WIDTH   100

class Logger {
private:
    static Logger*  instance;
    TeeBuf*         teeBuffer;
    TeeBuf*         teeBufferErr;
    std::ostream*   teeStream;
    std::ostream*   teeStreamErr;
    std::ofstream   logFile;
    std::streambuf* oldCoutStreamBuf;
    std::streambuf* oldCerrStreamBuf;
    std::string     filename;
    long            maxSize;
    int             logRotationCount;

    Logger(const std::string& filename, long maxLogSize = 10 * 1024 * 1024); // default max size: 10MB
    Logger(const Logger&);            // private copy constructor
    Logger& operator=(const Logger&); // private assignment operator
    void rotateLogs();
    std::string currentTimestamp();

public:
    enum LogLevel {
        DEBUG,
        DEBUG_DETAILED,
        DEBUG_CONFIG,
        INFO,
        WARN,
        ERROR,
        ERROR_INTERCEPTED,
        TRACE
    };

    static Logger*  getInstance(const std::string& filename = "logfile.txt", long maxLogSize = 10 * 1024 * 1024);
    static Logger*  getInstance(long maxLogSize);

    static          std::string generateLogFileName();
    static          std::string getLogFilePath(const std::string& fileName);
    static void     checkAndCreateLogDirectory(const std::string& dir);
    static void     cleanup();

    void            log(LogLevel level, const std::string& message, const std::string& color, const std::string& file, int line);
    void            log(LogLevel level, std::ostringstream &oss, const std::string& color, const std::string& file, int line);
    void            formatMessageByLogLevel(std::ostringstream& formattedMsg, LogLevel level, const std::string& file, int line );
    void            formatDebugMessage(LogLevel level, const std::string& message, std::ostringstream& formattedMsg, const std::string& color);
    void            captureStdout();
    void            releaseStdout();
    void            captureStderr();
    void            releaseStderr();
    void            logForCerr(const std::string& message);
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
 * While true macro overloading doesn't exist in C++, the use of different macro names (DEBUG_LOG and DEBUG_LOG_DETAILED) mimics function overloading behavior.
 */

#ifdef DEBUG_LEVEL

#define DEBUG_LOG(msg) Logger::getInstance()->log(Logger::DEBUG, msg, ANSI_WHITE , __FILE__, __LINE__)
#define DEBUG_LOG_LOC(msg, file, line) Logger::getInstance()->log(Logger::DEBUG, msg, ANSI_WHITE , file, line)

#if DEBUG_LEVEL >= 2
#define DEBUG_LOG_DETAILED(msg) Logger::getInstance()->log(Logger::DEBUG_DETAILED, msg, "", __FILE__, __LINE__)
#define DEBUG_LOG_DETAILED_LOC(msg, file, line) Logger::getInstance()->log(Logger::DEBUG_DETAILED, msg, "", file, line)

#define TRACE_LOG(msg) Logger::getInstance()->log(Logger::TRACE, msg, "", __FILE__, __LINE__)
#define TRACE_LOG_LOC(msg, file, line) Logger::getInstance()->log(Logger::TRACE, msg, "", file, line)

#define DEBUG_CONFIG(msg) Logger::getInstance()->log(Logger::DEBUG_CONFIG, msg, "", __FILE__, __LINE__)
#define DEBUG_CONFIG_LOC(msg, file, line) Logger::getInstance()->log(Logger::DEBUG_CONFIG, msg, "", file, line)
#else
#define DEBUG_LOG_DETAILED(msg)
#define DEBUG_LOG_DETAILED_LOC(msg, file, line)

#define TRACE_LOG(msg)
#define TRACE_LOG_LOC(msg, file, line)

#define DEBUG_CONFIG(msg)
#define DEBUG_CONFIG_LOC(msg, file, line)
#endif

#define INFO_LOG(msg) Logger::getInstance()->log(Logger::INFO, msg, ANSI_WHITE, __FILE__, __LINE__)
#define INFO_LOG_LOC(msg, file, line) Logger::getInstance()->log(Logger::INFO, msg, ANSI_WHITE, file, line)

#define WARN_LOG(msg) Logger::getInstance()->log(Logger::WARN, msg, ANSI_WHITE , __FILE__, __LINE__)
#define WARN_LOG_LOC(msg, file, line) Logger::getInstance()->log(Logger::WARN, msg, ANSI_WHITE , file, line)

#define ERROR_LOG(msg) Logger::getInstance()->log(Logger::ERROR, msg, ANSI_WHITE , __FILE__, __LINE__)
#define ERROR_LOG_LOC(msg, file, line) Logger::getInstance()->log(Logger::ERROR, msg, ANSI_WHITE , file, line)

#else

#define DEBUG_LOG(msg)
#define DEBUG_LOG_LOC(msg, file, line)

#define DEBUG_LOG_DETAILED(msg)
#define DEBUG_LOG_DETAILED_LOC(msg, file, line)

#define DEBUG_CONFIG(msg)
#define DEBUG_CONFIG_LOC(msg, file, line)

#define TRACE_LOG(msg)
#define TRACE_LOG_LOC(msg, file, line)

#define INFO_LOG(msg) Logger::getInstance()->log(Logger::INFO, msg, ANSI_WHITE, __FILE__, __LINE__)
#define INFO_LOG_LOC(msg, file, line) Logger::getInstance()->log(Logger::INFO, msg, ANSI_WHITE, file, line)

#define WARN_LOG(msg) Logger::getInstance()->log(Logger::WARN, msg, ANSI_WHITE, __FILE__, __LINE__)
#define WARN_LOG_LOC(msg, file, line) Logger::getInstance()->log(Logger::WARN, msg, ANSI_WHITE, file, line)

#define ERROR_LOG(msg) Logger::getInstance()->log(Logger::ERROR, msg, ANSI_WHITE, __FILE__, __LINE__)
#define ERROR_LOG_LOC(msg, file, line) Logger::getInstance()->log(Logger::ERROR, msg, ANSI_WHITE, file, line)

#endif


/*
********** USAGE **********
** DEBUG_LOG("This is a basic debug message.");
** DEBUG_LOG_DETAILED("This is a detailed debug message.");
*/

#endif