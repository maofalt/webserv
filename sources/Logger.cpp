/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/03/11 01:18:42 by rgarrigo          #+#    #+#             */
/*   Updated: 2023/08/27 17:32:04 by motero           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"

const int TIMESTAMP_WIDTH = 22;
const int LEVEL_WIDTH = 8;
const int FILE_FUNC_WIDTH = 25;
const int MESSAGE_WIDTH = 60;

std::string formatSection(const std::string& content, const std::string& color, int width) {
    std::ostringstream ss;
    ss << color << std::left << std::setw(width) << content << ANSI_RESET; // Use ANSI_RESET from the defined colors
    return ss.str();
}

std::string intToString(int value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

// Singleton instance initialization
Logger* Logger::instance = NULL;

// Private constructor to ensure only one instance is created
Logger::Logger(const std::string& fname, long maxLogSize) 
    : filename(fname),
    maxSize(maxLogSize),
    logRotationCount(0) {
    
    logFile.open(filename.c_str(), std::ios::app); // Open in append mode
    if (!logFile.is_open()) {
        throw std::runtime_error("Failed to open the log file.");
    }
}

Logger* Logger::getInstance(long maxLogSize) {
    if (!instance) {
        // Generate filename based on current date
        time_t rawtime;
        struct tm* timeinfo;
        char buffer[80];
        time(&rawtime);
        timeinfo = localtime(&rawtime);
        strftime(buffer, sizeof(buffer), "log_%Y-%m-%d.txt", timeinfo);
        
        std::string dir = "log";  // Define the directory name
        std::string path = dir + "/" + buffer;

        // Check if directory exists
        struct stat info;
        if (stat(dir.c_str(), &info) != 0) {
            // If not, create the directory
            #if defined(_WIN32)
            _mkdir(dir.c_str());
            #else
            mkdir(dir.c_str(), 0755);  // UNIX style permissions
            #endif
        }
        
        try {
            instance = new Logger(path, maxLogSize);
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
            return NULL;
        }
    }
    return instance;
}


Logger* Logger::getInstance(const std::string& fname, long maxLogSize) {
    if (!instance) {
        try {
            instance = new Logger(fname, maxLogSize);
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
            return NULL;
        }
    }
    return instance;
}

void Logger::captureStdout() {

    oldCoutStreamBuf = std::cout.rdbuf();
    try {
        teeBuffer = new TeeBuf(std::cout.rdbuf(), logFile.rdbuf());
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return;
    }

    try {
        teeStream = new std::ostream(teeBuffer);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return;
    }
    
    std::cout.rdbuf(teeStream->rdbuf());
}


// Restoring standard output to its original destination
void Logger::releaseStdout() {
    if (oldCoutStreamBuf)
        std::cout.rdbuf(oldCoutStreamBuf);
    oldCoutStreamBuf = NULL;

    if (teeStream)
        delete teeStream;
    teeStream = NULL;
    if (teeBuffer)
        delete teeBuffer;
    teeBuffer = NULL;
}

// Redirecting standard error (cerr) to our log file
void Logger::captureStderr() {
    oldCerrStreamBuf = std::cerr.rdbuf();

    try {
        teeBufferErr = new TeeBuf(std::cerr.rdbuf(), logFile.rdbuf());
    } catch (const std::exception& e) {
        std::cerr << "Error creating cerr TeeBuf: " << e.what() << std::endl;
        return;
    }

    try {
        teeStreamErr = new std::ostream(teeBufferErr);
    } catch (const std::exception& e) {
        std::cerr << "Error creating cerr TeeStream: " << e.what() << std::endl;
        return;
    }

    std::cerr.rdbuf(teeStreamErr->rdbuf());
}


// Restoring standard error to its original destination
void Logger::releaseStderr() {
    if (oldCerrStreamBuf)
        std::cerr.rdbuf(oldCerrStreamBuf);
    oldCerrStreamBuf = NULL;

    if (teeStreamErr) {
        delete teeStreamErr;
        teeStreamErr = NULL;
    }

    if (teeBufferErr) {
        delete teeBufferErr;
        teeBufferErr = NULL;
    }
}

// Implement rotateLogs to rotate log files when the current file exceeds maxSize
void Logger::rotateLogs() {
    logRotationCount++;
    logFile.close();
    for (int i = logRotationCount; i > 0; --i) {
        std::ostringstream ossOld, ossNew;
        ossOld << filename << "." << i;
        ossNew << filename << "." << (i + 1);
        
        std::string oldName = ossOld.str();
        std::string newName = ossNew.str();
        
        rename(oldName.c_str(), newName.c_str());
    }
    rename(filename.c_str(), (filename + ".1").c_str());
    logFile.open(filename.c_str(), std::ios_base::app);
}


// Implement the log function
void Logger::log(LogLevel level, const std::string& message, const std::string& file, int line) {
    // 1. Check log file size
    if (logFile.tellp() > maxSize) {
        rotateLogs();
    }

    // 2. Fetch the current timestamp
    std::string timestamp = currentTimestamp();

    // 3. Format and write the message to the log file or std::cout based on log level
    std::ostringstream formattedMsg;

    //formattedMsg << "[" << timestamp << "] ";
    formattedMsg << formatSection("[" + timestamp + "]", "", TIMESTAMP_WIDTH);
    
switch (level) {
    case DEBUG:
        formattedMsg << formatSection("[DEBUG]", ANSI_BOLD_WHITE, LEVEL_WIDTH);
        break;
    case DEBUG_DETAILED:
        formattedMsg << formatSection("[DEBUG]", ANSI_BOLD_WHITE, LEVEL_WIDTH);
        formattedMsg << formatSection(file + ":" + intToString(line) + ":" + __FUNCTION__, "", FILE_FUNC_WIDTH);
        break;
    case INFO:
        formattedMsg << formatSection("[INFO]", ANSI_BLUE, LEVEL_WIDTH);
        break;
    case WARN:
        formattedMsg << formatSection("[WARN]", ANSI_YELLOW, LEVEL_WIDTH);
        break;
    case ERROR:
        formattedMsg << formatSection("[ERROR]", ANSI_BOLD_RED, LEVEL_WIDTH);
        formattedMsg << formatSection(file + ":" + intToString(line) + ":" + __FUNCTION__, "", FILE_FUNC_WIDTH);
        break;
    case TRACE:
        formattedMsg << formatSection("[TRACE]", "", LEVEL_WIDTH);
        formattedMsg << "\n"; // New line for TRACE message content
        break;
}

    formattedMsg << formatSection(message, "", MESSAGE_WIDTH);



    #if DEBUG_LEVEL == 2
        if (level == DEBUG_DETAILED) {
            std::cout << formattedMsg.str() << std::endl;
        }
        if (level == TRACE) {
            std::cout << formattedMsg.str() << std::endl;
        }
    #endif
    
    #if DEBUG_LEVEL >= 1
        if (level == DEBUG) {
            std::cout << formattedMsg.str() << std::endl;
        }
    #endif
    
    if (level == INFO || level == WARN) {
        std::cout << formattedMsg.str() << std::endl;
    } 
    
    if (level == ERROR) {
        std::cerr << formattedMsg.str() << std::endl;
    }
}



// Implement currentTimestamp to get the current date and time as a string
std::string Logger::currentTimestamp() {
    time_t rawtime;
    struct tm * timeinfo;
    char buffer[80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
    return std::string(buffer);
}


void Logger::cleanup() {
    if (instance) {
        delete instance;
        instance = NULL; // or nullptr in modern C++
    }
}

Logger::~Logger() {
    if (logFile.is_open()) {
        logFile.close();
    }

    if (teeStream)
        delete teeStream;

    if (teeBuffer) 
        delete teeBuffer;

    if (teeStreamErr) {
        delete teeStreamErr;
    }

    if (teeBufferErr) {
        delete teeBufferErr;
    }

    if (oldCoutStreamBuf)
        std::cout.rdbuf(oldCoutStreamBuf);
    oldCoutStreamBuf = NULL;
    if (oldCerrStreamBuf)
        std::cerr.rdbuf(oldCerrStreamBuf);
    oldCerrStreamBuf = NULL;
    // Clean up the instance to prevent memory leak
}
