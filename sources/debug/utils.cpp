#include "Utils.hpp"

std::string intToStr(int num) {
    std::stringstream ss;
    ss << num;
    return ss.str();
}

void log_message_impl(Logger::LogLevel level, const char* file, int line, const char* format, ...) {
    va_list     args;
    char        buffer[4096];
    

    va_start(args, format);
    std::vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    std::string msg(buffer);
    
    switch(level) {
        case Logger::DEBUG: DEBUG_LOG_LOC(msg, file, line); break;
        case Logger::DEBUG_DETAILED: DEBUG_LOG_DETAILED_LOC(msg, file, line); break;
        case Logger::DEBUG_CONFIG: DEBUG_CONFIG_LOC(msg, file, line); break;
        case Logger::INFO: INFO_LOG_LOC(msg, file, line); break;
        case Logger::WARN: WARN_LOG_LOC(msg, file, line); break;
        case Logger::ERROR: ERROR_LOG_LOC(msg, file, line); break;
        case Logger::TRACE: TRACE_LOG_LOC(msg, file, line); break;
        default: break;
    }
}