/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   validateConfig.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/24 21:55:01 by rgarrigo          #+#    #+#             */
/*   Updated: 2023/09/05 16:53:51 by motero           ###   ########.fr       */
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
    if (!validateConfigData(_confData, "global")) {
        return false;
    }
    return validateMandatoryKeys(_confData, "global");
}

bool Config::validateVirtualServerConfig() {
    for (std::vector< ServerConfig >::iterator server_it = _servList.begin(); server_it != _servList.end(); ++server_it) {
        if (!validateConfigData(server_it->_servConfig, "server")) {
            return false;
        }

        // Now validate the locations inside this server
        //log_message(Logger::DEBUG, "Validating locations for server [%s]", server_it->_servConfig["server_name"][1].c_str());
        if (!validateLocationConfig(server_it->_locations)) {
            return false; // If location validation fails for one server, stop further validation
        }
    }
    return true;
}

bool Config::validateLocationConfig(std::vector<location>& locations) {
    for (std::vector<location>::iterator loc_it = locations.begin(); loc_it != locations.end(); ++loc_it) {
        if (!validateConfigData(loc_it->_locConfig, "location")) {
            return false;
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
            log_message(Logger::ERROR, "Multiple values found but multiple values are not allowed in section [%s]", values[0].c_str());

            // Keep only the first value and the first from the subvector.
            values.erase(values.begin() + 2, values.end()); // Remove all elements except the first two.
            if (!subVec.empty()) {
                values[1] = subVec[0];
            }
        }
    }
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

// Helper function to handle duplicate values and validate them
bool Config::validateConfigData(std::map<std::string, std::vector<std::string> >& confData, const std::string& contextType) {
    for (std::map<std::string, std::vector<std::string> >::iterator it = confData.begin(); it != confData.end(); ++it) {
        try {
            const std::map<std::string, std::string>& fieldProperties = getFieldProperties(contextType + "." + it->first);
            if (!fieldProperties.empty()) {
                handleDuplicateValues(it->second, fieldProperties);
            }
            std::vector<std::string> newVec(it->second.begin() + 1, it->second.end());
            validateValue(contextType + "." + it->first, newVec, fieldProperties);
        } catch (std::exception& e) {
            log_message(Logger::ERROR, "Failed to validate %s config key [%s]: %s", contextType.c_str(), it->first.c_str(), e.what());
            return false;
        }
    }
    return true;
}

// Helper function to ensure all mandatory contexts are present
bool Config::validateMandatoryKeys(const std::map<std::string, std::vector<std::string> >& confData, const std::string& contextType) {
    const std::set<std::string>& _mandatorySections = _validationFile->getMandatorySections();
    for (std::set<std::string>::const_iterator it = _mandatorySections.begin(); it != _mandatorySections.end(); ++it) {
        if(it->find(contextType + ".") != std::string::npos) {
            std::string parameter = it->substr(it->find(contextType + ".") + contextType.length() + 1);
            if (confData.find(parameter) == confData.end()) {
                log_message(Logger::ERROR, "Mandatory %s config key [%s] not found", contextType.c_str(), parameter.c_str());
                return false;
            }
        }
    }
    return true;
}