#include "TeeBuf.hpp"


TeeBuf::TeeBuf(std::streambuf* sb1, std::streambuf* sb2, void* o, void (Logger::*func)(const std::string&), bool shouldLog)
    : sb1(sb1), sb2(sb2), obj(o), log_func(func), logThroughLogger(shouldLog) {}

/**
* @brief Handles overflow condition for characters.
* 
* This method is invoked when a character needs to be written to the streambuf. The
* character is written to both sb1 and sb2.
* 
* @param c Character to be written.
* @return On success, returns the written character. On failure, returns EOF.
*/
int TeeBuf::overflow(int c) {
    if (logThroughLogger) {
        if (c != EOF) {
            buffered_str += static_cast<char>(c);
        }
        if (c == '\n' || c == EOF) {
            (reinterpret_cast<Logger*>(obj)->*log_func)(buffered_str); // Call the member function
            buffered_str.clear();
        }
        // Return after logging, without writing the raw message to cerr.
        return c;
    }
    
    // If not logging through logger, write to both buffers.
    int const r1 = sb1->sputc(c);
    int const r2 = sb2->sputc(c);
    return r1 == EOF || r2 == EOF ? EOF : c;
}

/**
 * @brief Synchronizes the state of the buffered sequence with the controlled sequence.
 * 
 * This method ensures that all buffered characters are transmitted to the controlled
 * sequence. It's called to flush the buffer, forwarding the flush request to both sb1 
 * and sb2.
 * 
 * @return On success, returns 0. On failure, returns -1.
 */
int TeeBuf::sync() {

    int const r1 = sb1->pubsync();
    int const r2 = sb2->pubsync();
    return r1 == 0 && r2 == 0 ? 0 : -1;
}
