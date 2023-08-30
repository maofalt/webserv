#ifndef INI_PARSER_HPP
#define INI_PARSER_HPP

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <cctype>     // for std::isspace
#include <algorithm>  // for std::find_if
#include <functional> // for std::not1 and std::ptr_fun
#include "Utils.hpp"


class IniParser {
private:
    std::map<std::string, std::map<std::string, std::string> > data;
    
    void trim(std::string& str);
    bool isCommentOrEmpty(const std::string& line) const;
    void handleSection(const std::string& line, std::string& currentSection);
    void handleKeyValuePair(const std::string& line, const std::string& currentSection);

public:
    IniParser();

    int loadConfig(const std::string& filename);

    // Function to fetch value given a section and key.
    bool getValue(const std::string& section, const std::string& key, std::string& value) const;

    // Function to fetch all keys and values in a section
    bool getSection(const std::string& section, std::map<std::string, std::string>& sectionData) const;

    // Debug function to print all sections, keys and values
    void printAll() const;
};

#endif // INI_PARSER_H
