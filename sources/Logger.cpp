/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/03/11 01:18:42 by rgarrigo          #+#    #+#             */
/*   Updated: 2023/08/24 23:17:32 by rgarrigo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"

#include "Logger.hpp"
#include <cassert>

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

Logger* Logger::getInstance(const std::string& fname, long maxLogSize) {
    if (!instance) {
        instance = new Logger(fname, maxLogSize);
    }
    return instance;
}

void Logger::captureStdout() {
    oldCoutStreamBuf = std::cout.rdbuf();
    teeBuffer = new TeeBuf(std::cout.rdbuf(), logFile.rdbuf());
    teeStream = new std::ostream(teeBuffer);
    std::cout.rdbuf(teeStream->rdbuf());
}


// Restoring standard output to its original destination
void Logger::releaseStdout() {
    std::cout.rdbuf(oldCoutStreamBuf);

    delete teeStream;
    delete teeBuffer;
}

// Redirecting standard error (cerr) to our log file
void Logger::captureStderr() {
    oldCerrStreamBuf = std::cerr.rdbuf(); // Save old buf
    std::cerr.rdbuf(logFile.rdbuf()); // Redirect cerr to our log file
}

// Restoring standard error to its original destination
void Logger::releaseStderr() {
    std::cerr.rdbuf(oldCerrStreamBuf);
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

    std::cout.rdbuf(oldCoutStreamBuf);
    // Clean up the instance to prevent memory leak
    if (instance) {
        delete instance;
        instance = NULL;
    }
}
