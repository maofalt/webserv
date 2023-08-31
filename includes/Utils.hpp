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
void            log_message(Logger::LogLevel level, const char* format, ...); 

#endif // UTILS_HPP