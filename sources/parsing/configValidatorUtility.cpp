/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   configValidatorUtility.cpp                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/24 21:55:01 by rgarrigo          #+#    #+#             */
/*   Updated: 2023/09/14 14:37:19 by motero           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigValidator.hpp"


std::map<std::string, std::string> ConfigValidator::getFieldProperties(const std::string& context) {
    std::map<std::string, std::string> sectionData;

    log_message(Logger::DEBUG, "Fetching data for context [%s]", context.c_str());
    if (_validationFile.getSection(context, sectionData)) {
        return sectionData;
    }
    
    return std::map<std::string, std::string>();
}

void ConfigValidator::handleDuplicateValues(std::vector<std::string>& values, 
                                            const std::map<std::string, std::string>& fieldProperties) {
    
    // Throw an exception if values vector is empty
    if (values.empty()) {
        throw std::runtime_error("No values found");
    }

    std::vector<std::string> subValues(values.begin() + 1, values.end());
    std::set<std::string> uniqueValues(subValues.begin(), subValues.end());
    
    // Early return if there are no duplicates
    if (uniqueValues.size() == subValues.size()) {
        return;
    }

    std::map<std::string, std::string>::const_iterator allowMultipleIt = fieldProperties.find("Multiple");
    bool allowMultiple = allowMultipleIt != fieldProperties.end() && allowMultipleIt->second == "true";
    
    if (allowMultiple) {
        handleAllowedMultipleValues(values, uniqueValues);
    } else {
        handleNotAllowedMultipleValues(values, subValues);
    }
}


void ConfigValidator::handleAllowedMultipleValues(std::vector<std::string>& values, const std::set<std::string>& uniqueValues) {
    log_message(Logger::WARN, "Multiple values found but multiple values are allowed. Replacing by unique values");

    values.erase(values.begin() + 1, values.end());
    values.insert(values.end(), uniqueValues.begin(), uniqueValues.end());
}

void ConfigValidator::handleNotAllowedMultipleValues(std::vector<std::string>& values, const std::vector<std::string>& subValues) {
    log_message(Logger::ERROR, "Multiple values found but multiple values are not allowed in section [%s]", values[0].c_str());
    
    values.erase(values.begin() + 2, values.end());
    if (!subValues.empty()) {
        values[1] = subValues[0];
    }
}


void ConfigValidator::logValidatedConfigKey(const std::string& contextType, const std::string& configKey) {
    log_message(Logger::DEBUG, "Validated %s config key [%s]", contextType.c_str(), configKey.c_str());
    log_message(Logger::DEBUG, "\t\tContext is [%s]", contextType.c_str());
    log_message(Logger::DEBUG, "\t\tConfig key is [%s]", configKey.c_str());
}


void ConfigValidator::applyGlobalSettings(const std::string& configKey, const std::vector<std::string>& configValues) {
    std::map<std::string, void(ConfigValidator::*)(const std::string&)>::iterator setterIt = _setterMap.find(configKey);
    if (setterIt != _setterMap.end()) {
        (this->*setterIt->second)(configValues[1]);
    }
}


void ConfigValidator::logConfigKeyValidationError(const std::string& contextType, const std::string& configKey, const char* error) {
    log_message(Logger::ERROR, "Failed to validate %s config key [%s]: %s", contextType.c_str(), configKey.c_str(), error);
}
