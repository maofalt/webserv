/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IniParser.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/30 16:11:41 by motero            #+#    #+#             */
/*   Updated: 2023/08/30 18:08:06 by motero           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "IniParser.hpp"

int IniParser::loadConfig(const std::string& filename) {
    std::ifstream   inFile(filename.c_str());
    std::string     line;
    std::string     currentSection;

    if (!inFile.is_open()) {
        log_message(Logger::ERROR, "Failed to open the INI file: %s\n", filename.c_str());
        return 1;
    }
    
    log_message(Logger::DEBUG, "Loading INI file: %s\n", filename.c_str());
    while (getline(inFile, line)) {
        trim(line);
        
        if (isCommentOrEmpty(line)) {
            continue;
        }

        if (line[0] == '[') {
            handleSection(line, currentSection);
        } else {
            handleKeyValuePair(line, currentSection);
        }
    }
    inFile.close();
    return 0;
}

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
        //log_message(Logger::TRACE, "Section: %s\n", currentSection.c_str());
        data[currentSection] = std::map<std::string, std::string>();
    }
}

void IniParser::handleKeyValuePair(const std::string& line, const std::string& currentSection) {
    size_t equalPos = line.find('=');
    
    if (equalPos != std::string::npos) {
        std::string key = line.substr(0, equalPos);
        std::string value = line.substr(equalPos + 1);
        
        trim(key);
        trim(value);
        //log_message(Logger::TRACE, "Key: %s, Value: %s\n", key.c_str(), value.c_str());
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
        
        //log_message(Logger::TRACE, "Section: %s\n", sectionIter->first.c_str());
        for (std::map<std::string, std::string>::const_iterator keyIter = sectionIter->second.begin(); keyIter != sectionIter->second.end(); ++keyIter) {
            iniFile += "\t" + keyIter->first + "=" + keyIter->second + "\n";
            //log_message(Logger::TRACE, "Key: %s, Value: %s\n", keyIter->first.c_str(), keyIter->second.c_str());
        }
        iniFile += "\n\n";
    }
    log_message(Logger::TRACE, "IniParser\n %s", iniFile.c_str());
}
