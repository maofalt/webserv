#ifndef UTILS_HPP
#define UTILS_HPP

#include <sstream>
#include <iostream>
#include <streambuf>
#include <cstdio>
#include <string>
#include <cstdarg>
#include "Logger.hpp"

//int to string
std::string     intToStr(int num);
void log_message_impl(Logger::LogLevel level, const char* file, int line, const char* format, ...);

#define log_message(level, format, ...) log_message_impl(level, __FILE__, __LINE__, format, ##__VA_ARGS__)

#endif // UTILS_HPP