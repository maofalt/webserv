#ifndef TEEBUF_HPP
#define TEEBUF_HPP

#include <iostream>
#include <streambuf>
#include <cstdio>
#include <string>

/*Forward decleration of Logger */
class Logger;

/**
 * @class TeeBuf
 * @brief A custom stream buffer that directs output to two streambuf objects.
 * 
 * TeeBuf acts as an intermediary between a C++ output stream and two other streambuf objects.
 * When data is written to TeeBuf, it is forwarded to both of the other streambuf objects.
 * This is analogous to the UNIX `tee` command, which allows output to be sent both to a file 
 * and to standard output. Using this class, we can simultaneously write to `cout` (or other
 * output streams) and to a log file, for example.
 */
class TeeBuf : public std::streambuf {
private:
    std::streambuf* sb1;
    std::streambuf* sb2;
    void*           obj;
    void            (Logger::*log_func)(const std::string&);
    std::string     buffered_str;
    bool            logThroughLogger;

protected:
    virtual int overflow(int c);
    virtual int sync();

public:
    TeeBuf(std::streambuf* sb1, std::streambuf* sb2, void* obj, void (Logger::*func)(const std::string&), bool shouldLog = false);
};

#endif // TEEBUF_HPP
