/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   loggerInstantiation.cpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/03/11 01:18:42 by rgarrigo          #+#    #+#             */
/*   Updated: 2023/09/14 15:35:10 by motero           ###   ########.fr       */
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

std::string Logger::generateLogFileName() {
    time_t rawtime;
    struct tm* timeinfo;
    char buffer[80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, sizeof(buffer), "log_%Y-%m-%d.txt", timeinfo);

    return std::string(buffer);
}

std::string Logger::getLogFilePath(const std::string& fileName) {
    std::string dir = "log";
    return dir + "/" + fileName;
}

void Logger::checkAndCreateLogDirectory(const std::string& dir) {
    struct stat info;
    if (stat(dir.c_str(), &info) != 0) {
        #if defined(_WIN32)
        _mkdir(dir.c_str());
        #else
        mkdir(dir.c_str(), 0755);
        #endif
    }
}

Logger* Logger::getInstance(long maxLogSize) {
    if (!instance) {
        std::string fileName = generateLogFileName();
        std::string filePath = getLogFilePath(fileName);

        checkAndCreateLogDirectory("log");
        
        try {
            instance = new Logger(filePath, maxLogSize);
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
        teeBuffer = new TeeBuf(std::cout.rdbuf(), logFile.rdbuf(), this, &Logger::logForCerr, false);
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

// Redirecting standard error (cerr) to our log file
void Logger::captureStderr() {
    oldCerrStreamBuf = std::cerr.rdbuf();

    try {
        teeBufferErr = new TeeBuf(std::cerr.rdbuf(), logFile.rdbuf(), this, &Logger::logForCerr, true);
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