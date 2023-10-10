/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parserIni.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/30 16:11:41 by motero            #+#    #+#             */
/*   Updated: 2023/10/02 14:45:17 by motero           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "IniParser.hpp"


int IniParser::loadConfig(const std::string& filename) {
    std::ifstream inFile(filename.c_str());
    std::string line, currentSection, previousSection;

    checkFileStatus(inFile);
    log_message(Logger::DEBUG, "Loading INI file: %s\n", filename.c_str());
    
    IniParser::initializeValidKeys();
    initializeValidationFunctions();
    initializeValidTypes();
    initializeRequireKeys();
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

//We encounter a new sencitona and we look if it is new or not
bool IniParser::isNewSection(const std::string& line) const {
    return (line[0] == '[' && line.find(']') != std::string::npos 
            &&  line.rfind(']') == line.find(']') && line.find(']') == line.size() - 1);
}

void IniParser::finalizePreviousSection(std::string& previousSection, std::string& currentSection) {
    if (!_keysInSection.empty()) {
        for (std::set<std::string>::const_iterator it = _mandatoryKeys.begin();
                it != _mandatoryKeys.end(); ++it) {
            if (_keysInSection.find(*it) == _keysInSection.end()) {
                log_message(Logger::WARN, "Missing mandatory key: %s in section: %s. Will skip the entire section.", it->c_str(), previousSection.c_str());
                _errorInSection = true;
                break;
            }
        
        }
    }
    //Create a list of mandatory section for later verification during the validation
    if (_keysInSection.find("Mandatory") != _keysInSection.end() && data[previousSection]["Mandatory"] == "true") {
        _mandatorySections.insert(previousSection);
    }
    _keysInSection.clear();
    
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
        if (data.find(currentSection) == data.end()) {
            data[currentSection] = std::map<std::string, std::string>();
        } else {
            log_message(Logger::WARN, "Duplicate section: %s. Will skip the entire section.", currentSection.c_str());
            _errorInSection = true;
            return ;
        }
    }
    _errorInSection = false;
    _currentType = "";
}

void IniParser::handleKeyValuePair(const std::string& line, const std::string& currentSection) {
    if (_errorInSection) {
        return;
    }
    
    //We look for the equal sign
    size_t equalPos = line.find('=');
    if (equalPos == std::string::npos) {
        logInvalidKeyValuePair(line, currentSection);
        return;
    }
    
    //We look for the key and the value
    std::string key = line.substr(0, equalPos);
    std::string value = line.substr(equalPos + 1);
    trim(key);
    trim(value);

    //We look if the key is duplicated
    if (isDuplicateKey(key, currentSection)) {
        logDuplicateKey(key, currentSection);
        return;
    }
    
    //We look if the key is valid
    if (key == "Type") {
        _currentType = value;
    }

    if (!isValidKey(key)) {
        logInvalidKey(key, currentSection);
        return;
    }
    //We look if the value is valid
    _keysInSection.insert(key);
    if (!isValidValueForKey(key, value)) {
        logInvalidValueForKey(value, key, currentSection);
        return;
    }

    //We add the key and the value to the map
    data[currentSection][key] = value;
}

bool IniParser::isDuplicateKey(const std::string& key, const std::string& currentSection) {
    return data[currentSection].find(key) != data[currentSection].end();
}

bool IniParser::isValidKey(const std::string& key) {
    return _validKeys.find(key) != _validKeys.end();
}