/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IniParser.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/30 16:11:41 by motero            #+#    #+#             */
/*   Updated: 2023/08/31 19:51:34 by motero           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "IniParser.hpp"

std::set<std::string>                                   IniParser::_validKeys;
std::map<std::string, IniParser::ValidationFunction>    IniParser::_validationFunctions;
std::string                                             IniParser::_currentType;
std::map<std::string, IniParser::ValidationFunction>    IniParser::_validTypes;

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
    initializeValidationFunctions();
    initializeValidTypes();
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
    _currentType = "";
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

        if (key == "Type") {
            _currentType = value;
        }
        
        if (_validKeys.find(key) == _validKeys.end()) {
            log_message(Logger::WARN, "Invalid key: %s in section %s. Will skip the entire section.", key.c_str(), currentSection.c_str());
            _errorInSection = true;
            return;
        }
        
        if (_validationFunctions.find(key) != _validationFunctions.end()) {
            if (!_validationFunctions[key](value)) {
                log_message(Logger::WARN, "Invalid value: %s for key: %s in section: %s.", value.c_str(), key.c_str(), currentSection.c_str());
                _errorInSection = true;
                return;
            }
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
    /*=====================================================================
                        VALIDATION TYPE METHODS
======================================================================*/
    std::map<std::string, std::map<std::string, std::string> >::const_iterator sectionIter = data.find(section);
    
    if (sectionIter != data.end()) {
        sectionData = sectionIter->second;
        return true;
    }
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

//Verify if the value is an integer, i mean an int number
//only numbers between 0 and 9
//integer must be between max int and min in
bool    IniParser::isInteger(const std::string& value) {
    std::string::const_iterator it = value.begin();
    while (it != value.end() && std::isdigit(*it)) ++it;
    return !value.empty() && it == value.end();
}

bool    IniParser::isMandatoryValid(const std::string& value) {
    return value == "true" || value == "false";
}

bool    IniParser::isMaxValid(const std::string& value) {
    //logic here
    (void)value;
    return true; // Placeholder
}

bool    IniParser::isMinValid(const std::string& value) {
    //logic here
    (void)value;
    return true; // Placeholder
}

bool IniParser::isValidationStrategyValid(const std::string& value) {
    std::set<std::string> validStrategies;
    
    validStrategies.insert("IsPositiveInteger");
    validStrategies.insert("IsInteger");
    validStrategies.insert("IsValidHostname");
    validStrategies.insert("UniqueList");
    validStrategies.insert("PathExistenceMap");
    validStrategies.insert("IsValidPatternList");
    validStrategies.insert("ListContainsValidMethods");
    validStrategies.insert("IsValidURLList");
    validStrategies.insert("PathExistenceList");
    validStrategies.insert("IsValidOnOffSettingList");
    validStrategies.insert("FileExistenceList");
    validStrategies.insert("IsValidFileTypesList");
    
    return validStrategies.find(value) != validStrategies.end();
}


/*=====================================================================
                        VALIDATION TYPE METHODS
======================================================================*/
void    IniParser::initializeValidTypes() {
    _validTypes["integer"] =    &IniParser::isValidInteger;
    _validTypes["string"] =     &IniParser::isValidString;
    _validTypes["bool"] =       &IniParser::isValidBoolean;
    _validTypes["ipv4"] =       &IniParser::isValidIpv4;
    _validTypes["port"] =       &IniParser::isValidPort;
}

bool    IniParser::isDefaultValid(const std::string& value) {
    
    if (_validTypes.find(_currentType) != _validTypes.end()) {
        return _validTypes[_currentType](value);
    }

    return true;
}

bool IniParser::isValidInteger(const std::string& value) {
    //test itha stringstream if the value is an integer
    std::stringstream ss(value);
    int i;
    ss >> i;
    if (ss.fail()) {
        ss.clear(); 
        return false;
    }
    return true;
}


bool IniParser::isValidString(const std::string& value) {
    (void)value;
    return true;
}

bool IniParser::isValidBoolean(const std::string& value) {
    return (value == "true"  || value == "false");
}

bool IniParser::isValidIpv4(const std::string& value) {
    std::istringstream ss(value);
    int a, b, c, d;
    char ch1, ch2, ch3;

    ss >> a >> ch1 >> b >> ch2 >> c >> ch3 >> d;

    if (!(ch1 == '.' && ch2 == '.' && ch3 == '.')) {
        return false;
    }

    if (a < 0 || a > 255 || b < 0 || b > 255 || c < 0 || c > 255 || d < 0 || d > 255) {
        return false;
    }

    return ss.eof();  // Make sure we consumed the whole string
}


bool IniParser::isValidPort(const std::string& value) {
    std::istringstream ss(value);
    int port;
    ss >> port;
    if (port < 0 || port > 65535) {
        return false;
    }
    return ss.eof();  // Make sure we consumed the whole string
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
