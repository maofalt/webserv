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
    std::cout.rdbuf(oldCoutStreamBuf);

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
    std::cerr.rdbuf(oldCerrStreamBuf);

    if (teeStreamErr) {
        delete teeStreamErr;
        teeStreamErr = NULL;
    }

    if (teeBufferErr) {
        delete teeBufferErr;
        teeBufferErr = NULL;
    }
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

    std::cout.rdbuf(oldCoutStreamBuf);
    std::cerr.rdbuf(oldCerrStreamBuf);
    // Clean up the instance to prevent memory leak
}
