/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigValidator.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/24 21:55:01 by rgarrigo          #+#    #+#             */
/*   Updated: 2023/09/06 17:46:06 by motero           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigValidator.hpp"

ConfigValidator::ConfigValidator(
    IniParser&                                          validationFile, 
    std::map<std::string, std::vector<std::string> >&   confData,
    std::vector<ServerConfig>&                          servList
) : _validationFile(validationFile), _confData(confData), _servList(servList) {
    
    // Setting the setter functions for globalconfig
    _setterMap["clientBodyLimit"]      =   &ConfigValidator::setClientBodyLimit;
    _setterMap["clientHeaderLimit"]    =   &ConfigValidator::setClientHeaderLimit;
    _setterMap["timeout"]              =   &ConfigValidator::setTimeout;
    _setterMap["maxConnections"]       =   &ConfigValidator::setMaxConnections;
    _setterMap["maxRequests"]          =   &ConfigValidator::setMaxRequests;
    _setterMap["maxRequestsPerIP"]     =   &ConfigValidator::setMaxRequestsPerIP;

}


ConfigValidator::~ConfigValidator() {
}

std::map<std::string, std::string> ConfigValidator::getFieldProperties(const std::string& context) {
    std::map<std::string, std::string> sectionData;

    log_message(Logger::DEBUG, "Fetching data for context [%s]", context.c_str());
    if (_validationFile.getSection(context, sectionData)) {
        return sectionData;
    }
    
    return std::map<std::string, std::string>();
}
const t_globalConfig& ConfigValidator::getGlobalConfig() const{
    return _globalConfig;
}

bool ConfigValidator::validateGlobalConfig() {
    if (!validateConfigData(_confData, "global")) {
        return false;
    }
    
    if (validateMandatoryKeys(_confData, "global"))
        return false; 

    return true;
}

bool ConfigValidator::validateVirtualServerConfig() {
    for (std::vector< ServerConfig >::iterator server_it = _servList.begin(); server_it != _servList.end(); ++server_it) {
        
        // Validate mandatory keys for virtual server
        if (!validateMandatoryKeys(server_it->_servConfig, "server")) {
            return false;
        }
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

bool ConfigValidator::validateLocationConfig(std::vector<location>& locations) {
    for (std::vector<location>::iterator loc_it = locations.begin(); loc_it != locations.end(); ++loc_it) {
        if (!validateMandatoryKeys(loc_it->_locConfig, "location")) {
            return false;
        }
        
        if (!validateConfigData(loc_it->_locConfig, "location")) {
            return false;
        }
    }
    return true;
}

void ConfigValidator::handleDuplicateValues(std::vector<std::string>& values, const std::map<std::string, std::string>& fieldProperties) {
    if (values.empty()) {
        throw std::runtime_error("No values found");
        return;
    }

    std::vector<std::string>    subVec(values.begin() + 1, values.end()); // Create a subvector excluding the first string.
    std::set<std::string>       uniqueValues(subVec.begin(), subVec.end()); // Check for unique values within the subvector.

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

bool ConfigValidator::validateConfig() {
    if (!validateGlobalConfig()) {
        return false;
    }
    if (!validateVirtualServerConfig()) {
        return false;
    }
    return true;
}

void ConfigValidator::validateValue(const std::string& fullContext, std::vector<std::string>& values, const std::map<std::string, std::string>& fieldProperties) {
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
bool ConfigValidator::validateConfigData(std::map<std::string, std::vector<std::string> >& confData, const std::string& contextType) {
    for (std::map<std::string, std::vector<std::string> >::iterator it = confData.begin(); it != confData.end(); ++it) {
        try {
            
            const std::map<std::string, std::string>& fieldProperties = getFieldProperties(contextType + "." + it->first);
            if (!fieldProperties.empty()) {
                handleDuplicateValues(it->second, fieldProperties);
            }
            
            std::vector<std::string> newVec(it->second.begin() + 1, it->second.end());
            validateValue(contextType + "." + it->first, newVec, fieldProperties);

            log_message(Logger::DEBUG, "Validated %s config key [%s]", contextType.c_str(), it->first.c_str());
            log_message(Logger::DEBUG, "\t\tContext is [%s]", contextType.c_str());
            log_message(Logger::DEBUG, "\t\tConfig key is [%s]", it->first.c_str());
            
            //fill struct global
            if (contextType == "global" && _setterMap.find(it->first) != _setterMap.end()) {
                (this->*_setterMap[it->first])(it->second[1]);
            }

        } catch (std::exception& e) {
            log_message(Logger::ERROR, "Failed to validate %s config key [%s]: %s", contextType.c_str(), it->first.c_str(), e.what());
            return false;
        }
    }
    return true;
}

// Helper function to ensure all mandatory contexts are present
bool ConfigValidator::validateMandatoryKeys(const std::map<std::string, std::vector<std::string> >& confData, const std::string& contextType) {
    const std::set<std::string>& _mandatorySections = _validationFile.getMandatorySections();
    
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

bool ConfigValidator::validateMandatoryKeys(const std::map<std::string, std::string>& confData, const std::string& contextType) {
    const std::set<std::string>& _mandatorySections = _validationFile.getMandatorySections();
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

void ConfigValidator::setClientBodyLimit(const std::string& value) {
    std::stringstream ss(value);
    ss >> _globalConfig.clientBodyLimit;
}

void ConfigValidator::setClientHeaderLimit(const std::string& value) {
    std::stringstream ss(value);
    ss >> _globalConfig.clientHeaderLimit;
}

void ConfigValidator::setTimeout(const std::string& value) {
    std::stringstream ss(value);
    ss >> _globalConfig.timeout;
}

void ConfigValidator::setMaxConnections(const std::string& value) {
    std::stringstream ss(value);
    ss >> _globalConfig.maxConnections;
}

void ConfigValidator::setMaxRequests(const std::string& value) {
    std::stringstream ss(value);
    ss >> _globalConfig.maxRequests;
}

void ConfigValidator::setMaxRequestsPerIP(const std::string& value) {
    std::stringstream ss(value);
    ss >> _globalConfig.maxRequestsPerIP;
    log_message(Logger::DEBUG, "Max requests per IP set to %d", _globalConfig.maxRequestsPerIP);
}



