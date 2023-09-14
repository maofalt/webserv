/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   iniParser.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/30 16:11:41 by motero            #+#    #+#             */
/*   Updated: 2023/09/14 14:43:00 by motero           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "IniParser.hpp"

std::set<std::string>                                   IniParser::_validKeys;
std::map<std::string, IniParser::ValidationFunction>    IniParser::_validationFunctions;
std::string                                             IniParser::_currentType;
std::map<std::string, IniParser::ValidationFunction>    IniParser::_validTypes;

void IniParser::initializeValidKeys() {
    _validKeys.insert("Multiple");
    _validKeys.insert("Type");
    _validKeys.insert("Mandatory");
    _validKeys.insert("Default");
    _validKeys.insert("Min");
    _validKeys.insert("Max");
    _validKeys.insert("Validation");
    _validKeys.insert("Description");
}

void IniParser::initializeRequireKeys() {
    _mandatoryKeys.insert("Multiple");
    _mandatoryKeys.insert("Type");
    _mandatoryKeys.insert("Mandatory");
    _mandatoryKeys.insert("Validation");
    _mandatoryKeys.insert("Description");
}

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

/*=====================================================================
                        VALIDATING PARSING METHODS
======================================================================*/

void IniParser::handleKeyValuePair(const std::string& line, const std::string& currentSection) {
    if (_errorInSection) {
        return;
    }
    
    size_t equalPos = line.find('=');
    if (equalPos == std::string::npos) {
        logInvalidKeyValuePair(line, currentSection);
        return;
    }
    
    std::string key = line.substr(0, equalPos);
    std::string value = line.substr(equalPos + 1);
    trim(key);
    trim(value);

    if (isDuplicateKey(key, currentSection)) {
        logDuplicateKey(key, currentSection);
        return;
    }

    if (key == "Type") {
        _currentType = value;
    }

    if (!isValidKey(key)) {
        logInvalidKey(key, currentSection);
        return;
    }
    _keysInSection.insert(key);
    if (!isValidValueForKey(key, value)) {
        logInvalidValueForKey(value, key, currentSection);
        return;
    }

    data[currentSection][key] = value;
}

bool IniParser::isDuplicateKey(const std::string& key, const std::string& currentSection) {
    return data[currentSection].find(key) != data[currentSection].end();
}

bool IniParser::isValidKey(const std::string& key) {
    return _validKeys.find(key) != _validKeys.end();
}

bool IniParser::isValidValueForKey(const std::string& key, const std::string& value) {
    if (_validationFunctions.find(key) == _validationFunctions.end()) {
        return true; // No validation function for this key, assume valid
    }
    return _validationFunctions[key](value);
}

void IniParser::logInvalidKeyValuePair(const std::string& line, const std::string& currentSection) {
    log_message(Logger::WARN, "Invalid key-value pair: %s in section: %s.", line.c_str(), currentSection.c_str());
    _errorInSection = true;
}

void IniParser::logDuplicateKey(const std::string& key, const std::string& currentSection) {
    log_message(Logger::WARN, "Duplicate key: %s in section: %s. Will skip the entire section.", key.c_str(), currentSection.c_str());
    _errorInSection = true;
}

void IniParser::logInvalidKey(const std::string& key, const std::string& currentSection) {
    log_message(Logger::WARN, "Invalid key: %s in section %s. Will skip the entire section.", key.c_str(), currentSection.c_str());
    _errorInSection = true;
}

void IniParser::logInvalidValueForKey(const std::string& value, const std::string& key, const std::string& currentSection) {
    log_message(Logger::WARN, "Invalid value: %s for key: %s in section: %s.", value.c_str(), key.c_str(), currentSection.c_str());
    _errorInSection = true;
}

/*=====================================================================
                        GETTERS
======================================================================*/                        

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
    log_message(Logger::DEBUG, "Looking for section: %s", section.c_str());
    std::map<std::string, std::map<std::string, std::string> >::const_iterator sectionIter = data.find(section);
    if (sectionIter != data.end()) {
        sectionData = sectionIter->second;
        log_message(Logger::DEBUG, "Found section: %s", section.c_str());
        return true;
    }
    log_message(Logger::WARN, "Section %s not found in the INI file.", section.c_str());
    return false;
}

/*=====================================================================
                     GENERAL VALIDATION METHODS
======================================================================*/
void    IniParser::initializeValidationFunctions() {
    _validationFunctions["Multiple"] =      &IniParser::isValidBoolean;
    _validationFunctions["Type"] =          &IniParser::isTypeValid;
    _validationFunctions["Mandatory"] =     &IniParser::isMandatoryValid;
    _validationFunctions["Default"] =       &IniParser::isDefaultValid; 
    _validationFunctions["Min"] =           &IniParser::isMinValid;
    _validationFunctions["Max"] =           &IniParser::isMaxValid;
    _validationFunctions["Validation"] =    &IniParser::isValidationStrategyValid;
    //"Description" key doesn't need validation.
}

bool    IniParser::isTypeValid(const std::string& value) {
    std::set<std::string> validTypes;
    validTypes.insert("integer");
    validTypes.insert("string");
    validTypes.insert("bool");
    validTypes.insert("list");
    validTypes.insert("map");
    validTypes.insert("ipv4");
    validTypes.insert("port");
    return validTypes.find(value) != validTypes.end();
}


bool    IniParser::isInteger(const std::string& value) {
    std::string::const_iterator it = value.begin();
    while (it != value.end() && std::isdigit(*it)) ++it;
    return !value.empty() && it == value.end();
}

bool    IniParser::isMandatoryValid(const std::string& value) {
    return value == "true" || value == "false";
}

bool IniParser::isValidationStrategyValid(const std::string& value) {
    std::set<std::string> validStrategies;
    
    validStrategies.insert("isPositiveInteger");
    validStrategies.insert("isInteger");
    validStrategies.insert("isValidServerName");
    validStrategies.insert("isValidDirectoryPath");
    validStrategies.insert("isValidLocPath");
    validStrategies.insert("isValidPath");
    validStrategies.insert("isValidMethod");
    validStrategies.insert("isValidRedirect");
    validStrategies.insert("isValidOnOff");
    validStrategies.insert("isValidCgiExtension");
    validStrategies.insert("isValidFileUploadTypes");
    return validStrategies.find(value) != validStrategies.end();
}




const std::set<std::string>& IniParser::getMandatorySections() const{
    if (_mandatorySections.empty()) {
        log_message(Logger::WARN, "No mandatory sections found in the INI file. Set is empty.");
        return _mandatorySections;
    }
    return _mandatorySections;
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
