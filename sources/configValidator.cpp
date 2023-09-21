/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   configValidator.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/24 21:55:01 by rgarrigo          #+#    #+#             */
/*   Updated: 2023/09/21 17:15:28 by motero           ###   ########.fr       */
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
    _setterMap["timeoutClient"]        =   &ConfigValidator::setTimeoutClient;
    _setterMap["timeoutCgi"]           =   &ConfigValidator::setTimeoutCgi;
    _setterMap["maxConnections"]       =   &ConfigValidator::setMaxConnections;
    _setterMap["maxRequests"]          =   &ConfigValidator::setMaxRequests;
    _setterMap["maxRequestsPerIP"]     =   &ConfigValidator::setMaxRequestsPerIP;

}

ConfigValidator::~ConfigValidator() {
}

ConfigValidator &ConfigValidator::operator=(ConfigValidator const &rhs)
{
    _validationFile = rhs._validationFile;
    _confData = rhs._confData;
    _servList = rhs._servList;
    _globalConfig = rhs._globalConfig;
    _setterMap = rhs._setterMap;
    return (*this);
}


const t_globalConfig& ConfigValidator::getGlobalConfig() const{
    return _globalConfig;
}

bool ConfigValidator::validateGlobalConfig() {
    if (!validateConfigData(_confData, "global")) {
        log_message(Logger::ERROR, "Global config validation failed");
        return false;
    }
    
    if (!validateMandatoryKeys(_confData, "global")) {
        log_message(Logger::WARN, "Mandatory keys for global config not found");
        return false; 
    }
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



bool ConfigValidator::validateConfig() {
    if (!validateGlobalConfig()) {
        log_message(Logger::ERROR, "Global config validation failed");
        return false;
    }
    
    if (!validateVirtualServerConfig()) {
        log_message(Logger::ERROR, "One of virtual server config validation failed");
        return false;
    }
    return true;
}

void ConfigValidator::validateValue(const std::string& fullContext, std::vector<std::string>& values, const std::map<std::string, std::string>& fieldProperties) {
    try {
        ValidationStrategy* strategy = getValidationStrategy(fullContext, fieldProperties);
        if (!strategy) {
            log_message(Logger::ERROR, "No validation strategy found for context [%s]", fullContext.c_str());
            return;
        }

        logValuesToValidate(values);

        validateValuesUsingStrategy(values, fieldProperties, strategy, fullContext);
        
    } catch (std::exception& e) {
        log_message(Logger::ERROR, "Error during context [%s], value [%s] validation: %s", fullContext.c_str(), values[0].c_str(), e.what());
        throw; 
    }
}

ValidationStrategy* ConfigValidator::getValidationStrategy(const std::string& fullContext, const std::map<std::string, std::string>& fieldProperties) {
    ValidationFactory& factory = ValidationFactory::getInstance();

    std::map<std::string, std::string>::const_iterator strategyIt = fieldProperties.find("Validation");
    if (strategyIt == fieldProperties.end()) {
        log_message(Logger::ERROR, "Validation type not found for context [%s]", fullContext.c_str());
        return NULL;
    }
    return factory.getStrategy(strategyIt->second);
}

void ConfigValidator::logValuesToValidate(const std::vector<std::string>& values) {
    log_message(Logger::DEBUG, "Values to validate:");
    for (std::vector<std::string>::const_iterator it = values.begin(); it != values.end(); ++it) {
        log_message(Logger::DEBUG, "Value: %s", it->c_str());
    }
}

void ConfigValidator::validateValuesUsingStrategy(const std::vector<std::string>& values, const std::map<std::string, std::string>& fieldProperties, ValidationStrategy* strategy, const std::string& fullContext) {
    for (std::vector<std::string>::const_iterator it = values.begin(); it != values.end(); ++it) {
        strategy->validate(*it, fieldProperties);
        log_message(Logger::DEBUG, "[YES] Validation passed for context [%s], value [%s]", fullContext.c_str(), it->c_str());
    }
}


bool ConfigValidator::validateConfigData(std::map<std::string, std::vector<std::string> >& confData, const std::string& contextType) {
    for (std::map<std::string, std::vector<std::string> >::iterator it = confData.begin(); it != confData.end(); ++it) {
        const std::string fullContext = contextType + "." + it->first;
        
        try {
            const std::map<std::string, std::string>& fieldProperties = getFieldProperties(fullContext);
            if (fieldProperties.empty()) {
                continue;
            }
            
            handleDuplicateValues(it->second, fieldProperties);
            
            std::vector<std::string> newValueVector(it->second.begin() + 1, it->second.end());
            validateValue(fullContext, newValueVector, fieldProperties);

            logValidatedConfigKey(contextType, it->first);
            
            if (contextType == "global") {
                applyGlobalSettings(it->first, it->second);
            }
        } catch (std::exception& e) {
            logConfigKeyValidationError(contextType, it->first, e.what());
            return false;
        }
    }
    return true;
}

// Helper function to ensure all mandatory contexts are present
bool ConfigValidator::validateMandatoryKeys(std::map<std::string, std::vector<std::string> >& confData, const std::string& contextType) {
    const std::set<std::string>& _mandatorySections = _validationFile.getMandatorySections();
    
    for (std::set<std::string>::const_iterator it = _mandatorySections.begin(); it != _mandatorySections.end(); ++it) {
        if(it->find(contextType + ".") != std::string::npos) {
            
            std::string parameter = it->substr(it->find(contextType + ".") + contextType.length() + 1);
            if (confData.find(parameter) == confData.end()) {
                log_message(Logger::ERROR, "Mandatory %s config key [%s] not found", contextType.c_str(), parameter.c_str());
                // server.clientBodyLimit i no found create  it wiht the value of globaclientBodyLimit
                std::string globalKey = contextType + "." + parameter;
                if ( globalKey == "server.clientBodyLimit") {
                    log_message(Logger::INFO, "Creating server.clientBodyLimit with value of global.clientBodyLimit");
                    std::stringstream ss;
                    ss << _globalConfig.clientBodyLimit;                    
                    std::vector<std::string> newValueVector;
                    log_message(Logger::INFO, "\t\t parameter is = %s", parameter.c_str());
                    newValueVector.push_back(parameter);
                    newValueVector.push_back(ss.str());
                    //Create inside map server.clientBodyLimit and push newValueVector
                    confData.insert(std::pair<std::string, std::vector<std::string> >(parameter, newValueVector));
                    log_message(Logger::INFO, "server.clientBodyLimit created");
                    return true;
                }
                return false;
            }
        }
    }
    return true;
}

bool ConfigValidator::validateMandatoryKeys( std::map<std::string, std::string>& confData, const std::string& contextType) {
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



