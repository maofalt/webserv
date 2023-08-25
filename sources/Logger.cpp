/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/03/11 01:18:42 by rgarrigo          #+#    #+#             */
/*   Updated: 2023/08/25 17:38:40 by motero           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"


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

    formattedMsg << "[" << timestamp << "] " << "[FILE: " << file << "] " << "[LINE: " << line << "] ";

    switch (level) {
        case DEBUG:
            formattedMsg << "[DEBUG] ";
            break;
        case DEBUG_DETAILED:
            formattedMsg << "[DEBUG_DETAILED] ";
            break;
        case INFO:
            formattedMsg << "[INFO] ";
            break;
        case WARN:
            formattedMsg << "[WARN] ";
            break;
        case ERROR:
            formattedMsg << "[ERROR] ";
            break;
    }
    formattedMsg << message;

    // Depending on the log level, print to different destinations
    if (level == DEBUG || level == DEBUG_DETAILED) {
        std::cout << formattedMsg.str() << std::endl;
    }
    //std::cout << formattedMsg.str() << std::endl;
    logFile << formattedMsg.str() << std::endl;
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
