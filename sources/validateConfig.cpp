/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   validateConfig.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/24 21:55:01 by rgarrigo          #+#    #+#             */
/*   Updated: 2023/09/04 19:09:18 by motero           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"

std::map<std::string, std::string> Config::getFieldProperties(const std::string& context) {
    std::map<std::string, std::string> sectionData;

    log_message(Logger::DEBUG, "Fetching data for context [%s]", context.c_str());
    if (_validationFile->getSection(context, sectionData)) {
        return sectionData;
    }
    
    return std::map<std::string, std::string>();
}


bool Config::validateGlobalConfig() {
    // Validate existing global contexts
    for (std::map<std::string, std::vector<std::string> >::iterator it = _confData.begin(); it != _confData.end(); ++it) {
        try {         
            // Check individually each global context
            const std::map<std::string, std::string>& fieldProperties = getFieldProperties("global." + it->first);
            
            handleDuplicateValues(it->second, fieldProperties);
            
            std::vector<std::string> newVec(it->second.begin() + 1, it->second.end());
            
            validateValue("global." + it->first, newVec, fieldProperties);
        } catch (std::exception& e) {
            log_message(Logger::ERROR, "Failed to validate global config key [%s]: %s", it->first.c_str(), e.what());
            return false;
        }
    }

    // Check if all mandatory global context are present
    const std::set<std::string>& _mandatorySections = _validationFile->getMandatorySections();
    for (std::set<std::string>::iterator it = _mandatorySections.begin(); it != _mandatorySections.end(); ++it) {
        if(it->find("global.") != std::string::npos) {
            std::string parameter = it->substr(it->find("global.") + std::string("global.").length());
            if (_confData.find(parameter) == _confData.end()) {
                log_message(Logger::ERROR, "Mandatory global config key [%s] not found", parameter.c_str());
                return false;
            }
        }
    }
    
    return true;
}

void Config::handleDuplicateValues(std::vector<std::string>& values, const std::map<std::string, std::string>& fieldProperties) {
    if (values.empty()) {
        throw std::runtime_error("No values found");
        return;
    }

    std::vector<std::string> subVec(values.begin() + 1, values.end()); // Create a subvector excluding the first string.
    std::set<std::string> uniqueValues(subVec.begin(), subVec.end()); // Check for unique values within the subvector.

    std::map<std::string, std::string>::const_iterator multipleIt = fieldProperties.find("Multiple");
    
    if (uniqueValues.size() != subVec.size()) { // If duplicates found.
        if (multipleIt != fieldProperties.end() && multipleIt->second == "true") {
            log_message(Logger::WARN, "Multiple values found but multiple values are allowed. Replacing by unique values");

            // Keep the first value and assign the unique values after it.
            values.erase(values.begin() + 1, values.end()); // Remove all elements except the first.
            for (std::set<std::string>::iterator it = uniqueValues.begin(); it != uniqueValues.end(); ++it) {
                values.push_back(*it);
            }
        } else {
            log_message(Logger::ERROR, "Multiple values found but multiple values are not allowed");

            // Keep only the first value and the first from the subvector.
            values.erase(values.begin() + 2, values.end()); // Remove all elements except the first two.
            if (!subVec.empty()) {
                values[1] = subVec[0];
            }
        }
    }
}


bool Config::validateVirtualServerConfig() {
    for (std::vector< ServerConfig >::iterator server_it = _servList.begin(); server_it != _servList.end(); ++server_it) {
        for (std::map<std::string, std::vector<std::string> >::iterator it = server_it->_servConfig.begin(); it != server_it->_servConfig.end(); ++it) {
            try {
                std::vector<std::string> newVec(it->second.begin() + 1, it->second.end());
                validateValue("server." + it->first, newVec, getFieldProperties("server." + it->first));
            } catch (std::exception& e) {
                log_message(Logger::ERROR, "Failed to validate server config key [%s]: %s", it->first.c_str(), e.what());
                return false; // Stop further validation if one fails
            }
        }
        // Now validate the locations inside this server
        if (!validateLocationConfig(server_it->_locations)) {
            return false; // If location validation fails for one server, stop further validation
        }
    }
    return true; 
}

bool Config::validateLocationConfig(std::vector<location>& locations) {
    for (std::vector<location>::iterator loc_it = locations.begin(); loc_it != locations.end(); ++loc_it) {
        for (std::map<std::string, std::vector<std::string> >::iterator it = loc_it->_locConfig.begin(); it != loc_it->_locConfig.end(); ++it) {
            try {
                validateValue(it->first, it->second, getFieldProperties("location." + it->first));
            } catch (std::exception& e) {
                log_message(Logger::ERROR, "Failed to validate location config key [%s]: %s", it->first.c_str(), e.what());
                return false; // Stop further validation if one fails
            }
        }
    }
    return true; // All validations passed
}

bool Config::validateConfig() {
    if (!validateGlobalConfig()) {
        return false;
    }
    if (!validateVirtualServerConfig()) {
        return false;
    }
    return true;
}

void Config::validateValue(const std::string& fullContext, std::vector<std::string>& values, const std::map<std::string, std::string>& fieldProperties) {
    validationFactory& factory = validationFactory::getInstance();
    ValidationStrategy* strategy = NULL;
    
    try {
        // Extracting the validation type from the field properties
        std::map<std::string, std::string>::const_iterator strategyIt = fieldProperties.find("Validation");
        if (strategyIt == fieldProperties.end()) {
            log_message(Logger::ERROR, "Validation type not found for context [%s]", fullContext.c_str());
            return;
        }
                
        // Getting the strategy using the extracted validation type
        strategy = factory.getStrategy(strategyIt->second);

        log_message(Logger::DEBUG, "Values to validate:");
        for (std::vector<std::string>::const_iterator it = values.begin(); it != values.end(); ++it) {
            log_message(Logger::DEBUG, "Value: %s", it->c_str());
        }

        if (strategy) {
            for (std::vector<std::string>::const_iterator it = values.begin(); it != values.end(); ++it) {
                strategy->validate(*it, fieldProperties);
                log_message(Logger::DEBUG, "[YES] Validation passed for context [%s], value [%s]", fullContext.c_str(), it->c_str());
            }
        } else {
            log_message(Logger::ERROR, "No validation strategy found for context [%s]", fullContext.c_str());
        }
    } catch (std::exception& e) {
        log_message(Logger::ERROR, "Error during context [%s], value [%s] validation: %s", fullContext.c_str(), values[0].c_str(), e.what());
        throw; 
    }
}