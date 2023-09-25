/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   logger.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/03/11 01:18:42 by rgarrigo          #+#    #+#             */
/*   Updated: 2023/09/14 16:19:43 by motero           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"


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

std::vector<std::string> splitIntoLines(const std::string& str, size_t width) {
    std::vector<std::string> result;
    std::istringstream stream(str);
    std::string line;

    width -= 1;  // For the added whitespace

    while (std::getline(stream, line)) {
        size_t current = 0, lastSpace = 0;

        for (size_t i = 0; i < line.size(); ++i) {
            if (line[i] == ' ') {
                lastSpace = i;
            }

            if (i - current >= width) {
                if (lastSpace == current) {
                    result.push_back(line.substr(current, width));
                    current = i;
                } else {
                    result.push_back(line.substr(current, lastSpace - current));
                    current = lastSpace + 1;
                }
            }
        }

        if (current < line.size()) {
            result.push_back(line.substr(current));
        }
    }

    return result;
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

void Logger::formatMessageByLogLevel(std::ostringstream& formattedMsg, LogLevel level, const std::string& file, int line) {
    switch (level) {
       case DEBUG:
           formattedMsg << formatSection("[DEBUG]", ANSI_BOLD_WHITE, LEVEL_WIDTH);
           break;
       case DEBUG_DETAILED:
           formattedMsg << formatSection("[DEBUG]", ANSI_BOLD_WHITE, LEVEL_WIDTH);
           formattedMsg << formatSection(file + ":" + intToString(line) + ":" + __FUNCTION__, ANSI_BOLD_RED, FILE_FUNC_WIDTH);
           break;
       case DEBUG_CONFIG:
           formattedMsg << formatSection("[CONFIG]", ANSI_BOLD_WHITE, LEVEL_WIDTH);
           break;
       case INFO:
           formattedMsg << formatSection("[INFO]", ANSI_BLUE, LEVEL_WIDTH);
           break;
       case WARN:
           formattedMsg << formatSection("[WARN]", ANSI_YELLOW, LEVEL_WIDTH);
           break;
       case ERROR:
           formattedMsg << formatSection("[ERROR]", ANSI_BOLD_RED, LEVEL_WIDTH);
           formattedMsg << formatSection(file + ":" + intToString(line) + ":" + __FUNCTION__, ANSI_BOLD_RED, FILE_FUNC_WIDTH);
           break;
       case ERROR_INTERCEPTED:
           formattedMsg << formatSection("[ERROR]", ANSI_BOLD_RED, LEVEL_WIDTH);
           break;
       case TRACE:
           formattedMsg << formatSection("[TRACE]", "", LEVEL_WIDTH);
           formattedMsg << "\n"; // New line for TRACE message content
           break;
    }
}

void Logger::formatDebugMessage(LogLevel level, const std::string& message, std::ostringstream& formattedMsg, const std::string& color) {
    if (level == TRACE || level == DEBUG_CONFIG) {
    std::vector<std::string> lines = splitIntoLines(message, MESSAGE_WIDTH);
    std::cout << formattedMsg.str() << std::endl;
    std::cout << SEPARATOR_START << std::endl;
    for (size_t i = 0; i < lines.size(); ++i) {
        formattedMsg.str(""); // Clear the contents of the stream
        formattedMsg << " " << formatSection(lines[i], color, MESSAGE_WIDTH);  // Add a space before the message
        std::cout << formattedMsg.str() << std::endl;
    }
    std::cout << SEPARATOR_END << std::endl;
    std::cout << std::endl;
    } else if (level == DEBUG_DETAILED) {
        formattedMsg << formatSection(message,  color , MESSAGE_WIDTH);
        std::cout << formattedMsg.str() << std::endl;}
}

// Implement the log function
void Logger::log(LogLevel level, const std::string& message, const std::string& color, const std::string& file, int line) {
    // 1. Check log file size
    if (logFile.tellp() > maxSize) {
        rotateLogs();
    }
    // 2. Fetch the current timestamp
    std::string timestamp = currentTimestamp();
    // 3. Format and write the message to the log file or std::cout based on log level
    std::ostringstream formattedMsg;
    formattedMsg << formatSection("[" + timestamp + "]", ANSI_CYAN, TIMESTAMP_WIDTH);
    //4-Format logLEVEL depending on log level
    formatMessageByLogLevel(formattedMsg, level, file, line);
    //5-Format message depending on log level
    #if DEBUG_LEVEL == 2
    formatDebugMessage(level, message, formattedMsg, color);
    #endif
    //6-Format message depending on log level
    formattedMsg << formatSection(message,  color , MESSAGE_WIDTH);
    
    #if DEBUG_LEVEL >= 1
        if (level == DEBUG) {
            std::cout << formattedMsg.str() << std::endl;
        }
    #endif
    
    if (level == INFO || level == WARN || level == ERROR || level == ERROR_INTERCEPTED) {
        std::cout << formattedMsg.str() << std::endl;
    } 
}

void    Logger::log(LogLevel level, std::ostringstream &oss, const std::string& color, const std::string& file, int line) {
    std::string message = oss.str();
    log(level, message, color, file, line);
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

void     Logger::logForCerr(const std::string& message) {
    log(ERROR_INTERCEPTED, message, "",  "Intercepted Cerr" , -1);
}