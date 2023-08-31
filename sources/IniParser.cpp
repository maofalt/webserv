/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IniParser.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/30 16:11:41 by motero            #+#    #+#             */
/*   Updated: 2023/08/31 15:39:05 by motero           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "IniParser.hpp"

std::set<std::string> IniParser::_validKeys;

void IniParser::initializeValidKeys() {
    _validKeys.insert("Type");
    _validKeys.insert("Mandatory");
    _validKeys.insert("Default");
    _validKeys.insert("Min");
    _validKeys.insert("Max");
    _validKeys.insert("Validation");
    _validKeys.insert("Description");
}

int IniParser::loadConfig(const std::string& filename) {
    std::ifstream inFile(filename.c_str());
    std::string line, currentSection, previousSection;

    checkFileStatus(inFile);
    log_message(Logger::DEBUG, "Loading INI file: %s\n", filename.c_str());
    
    IniParser::initializeValidKeys();
    _errorInSection = false;

    while (getline(inFile, line)) {
        trim(line);

        if (isCommentOrEmpty(line)) continue;

        if (isNewSection(line)) {
            finalizePreviousSection(previousSection, currentSection);
            handleSection(line, currentSection);
            previousSection = currentSection;
        } else {
            handleKeyValuePair(line, currentSection);
        }
    }
    
    finalizeLoading(previousSection, currentSection);

    inFile.close();
    return 0;
}

void IniParser::checkFileStatus(std::ifstream& file) const {
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open the INI file");
        // No need for return 1 after a throw, it will exit the function.
    }
    
    if (file.peek() == std::ifstream::traits_type::eof()) {
        throw std::runtime_error("INI file is empty");
    }
}

bool IniParser::isNewSection(const std::string& line) const {
    return line[0] == '[';
}

void IniParser::finalizePreviousSection(std::string& previousSection, std::string& currentSection) {
    if (_errorInSection) {
        data.erase(currentSection);
        _errorInSection = false;
    }
    checkAndEraseEmptySection(previousSection);
}

void IniParser::checkAndEraseEmptySection(const std::string& section) {
    if (!section.empty() && data[section].empty()) {
        log_message(Logger::WARN, "Section %s is empty. Erasing section.", section.c_str());
        data.erase(section);
    }
}

void IniParser::finalizeLoading(const std::string& previousSection, const std::string& currentSection) {
    if (_errorInSection) {
        data.erase(currentSection);
    }
    checkAndEraseEmptySection(previousSection);
}

// int IniParser::loadConfig(const std::string& filename) {
//     std::ifstream   inFile(filename.c_str());
//     std::string     line;
//     std::string     currentSection;
//     std::string     previousSection;

//     if (!inFile.is_open()) {
//         throw std::runtime_error("Failed to open the INI file");
//         return 1;
//     }
    
//     if (inFile.peek() == std::ifstream::traits_type::eof()) {
//         throw std::runtime_error("INI file is empty");
//         return 1;
//     }
    
//     log_message(Logger::DEBUG, "Loading INI file: %s\n", filename.c_str());
//     IniParser::initializeValidKeys();
//     _errorInSection = false;
//     while (getline(inFile, line)) {
//         trim(line);
        
//         if (isCommentOrEmpty(line)) {
//             continue;
//         }

//         if (line[0] == '[') {
//             // Before starting a new section, check if there was an error in the previous section
//             if (_errorInSection) {
//                 data.erase(currentSection);
//                 currentSection.clear();
//                 _errorInSection = false;
//             }
            
//             if (!previousSection.empty() && data[previousSection].empty()) {
//                 log_message(Logger::WARN, "Section %s is empty. Erasing section.", previousSection.c_str());
//                 data.erase(previousSection);
//             }
//             previousSection = currentSection;
//             handleSection(line, currentSection);
//         } else {
//             handleKeyValuePair(line, currentSection);
//         }
//     }

//     // Check one last time after the loop ends
//     if (_errorInSection) {
//         data.erase(currentSection);
//         currentSection.clear();
//     }
//     if (!previousSection.empty() && data[previousSection].empty()) {
//         log_message(Logger::WARN, "Section %s is empty. Erasing section.", previousSection.c_str());
//         data.erase(previousSection);
//     }
    
//     inFile.close();
//     return 0;
// }

IniParser::IniParser() {
}


void IniParser::trim(std::string& str) {
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
    str.erase(std::find_if(str.rbegin(), str.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), str.end());
}

bool IniParser::isCommentOrEmpty(const std::string& line) const {
    return line.empty() || line[0] == ';';
}

void IniParser::handleSection(const std::string& line, std::string& currentSection) {
    size_t end = line.find(']');
    
    if (end != std::string::npos) {
        currentSection = line.substr(1, end - 1);
        data[currentSection] = std::map<std::string, std::string>();
    }
    _errorInSection = false;
}

void IniParser::handleKeyValuePair(const std::string& line, const std::string& currentSection) {
    if (_errorInSection) {
        // If there's an error in the current section, skip processing further key-value pairs
        return;
    }
    
    size_t equalPos = line.find('=');
    
    if (equalPos != std::string::npos) {
        std::string key = line.substr(0, equalPos);
        std::string value = line.substr(equalPos + 1);
        
        trim(key);
        trim(value);

        if (_validKeys.find(key) == _validKeys.end()) {
            log_message(Logger::WARN, "Invalid key: %s in section %s. Will skip the entire section.", key.c_str(), currentSection.c_str());
            _errorInSection = true;
            return;
        }
    
        data[currentSection][key] = value;
    }
}


bool IniParser::getValue(const std::string& section, const std::string& key, std::string& value) const {
    
    std::map<std::string, std::map<std::string, std::string> >::const_iterator sectionIter = data.find(section);
    
    if (sectionIter != data.end()) {
        
        std::map<std::string, std::string>::const_iterator keyIter = sectionIter->second.find(key);
        if (keyIter != sectionIter->second.end()) {
            value = keyIter->second;
            return true;
        }
    }
    return false;
}

bool IniParser::getSection(const std::string& section, std::map<std::string, std::string>& sectionData) const {
    
    std::map<std::string, std::map<std::string, std::string> >::const_iterator sectionIter = data.find(section);
    
    if (sectionIter != data.end()) {
        sectionData = sectionIter->second;
        return true;
    }
    return false;
}


void IniParser::printAll() const {
    
    std::string iniFile;
    
    for (std::map<std::string, std::map<std::string, std::string> >::const_iterator sectionIter = data.begin();
        sectionIter != data.end(); ++sectionIter) {
        iniFile += "[" + sectionIter->first + "]" + "\n";
        
        for (std::map<std::string, std::string>::const_iterator keyIter = sectionIter->second.begin(); keyIter != sectionIter->second.end(); ++keyIter) {
            iniFile += "\t" + keyIter->first + "=" + keyIter->second + "\n";
        }
        iniFile += "\n\n";
    }
    log_message(Logger::TRACE, "IniParser\n %s", iniFile.c_str());
}
