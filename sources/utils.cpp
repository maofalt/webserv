#include "Utils.hpp"

std::string intToStr(int num) {
    std::stringstream ss;
    ss << num;
    return ss.str();
}

void    log_message(Logger::LogLevel level, const char* format, ...) {
    va_list     args;
    char    buffer[4096];
    

    va_start(args, format);
    std::vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    std::string msg(buffer);
    
    switch(level) {
        case Logger::DEBUG: DEBUG_LOG(msg); break;
        case Logger::DEBUG_DETAILED: DEBUG_LOG_DETAILED(msg); break;
        case Logger::DEBUG_CONFIG: DEBUG_CONFIG(msg); break;
        case Logger::INFO: INFO_LOG(msg); break;
        case Logger::WARN: WARN_LOG(msg); break;
        case Logger::ERROR: ERROR_LOG(msg); break;
        case Logger::TRACE: TRACE_LOG(msg); break;
        default: break;
    }
}