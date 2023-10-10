/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   iniParser.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/30 16:11:41 by motero            #+#    #+#             */
/*   Updated: 2023/09/14 15:06:57 by motero           ###   ########.fr       */
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

IniParser::IniParser() {
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
