/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   validationFactory.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/24 21:55:01 by rgarrigo          #+#    #+#             */
/*   Updated: 2023/09/03 17:21:24 by motero           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"

std::map<std::string, std::string> Config::getFieldProperties(const std::string& context) {
    std::map<std::string, std::string> sectionData;

    log_message(Logger::DEBUG, "Fetching data for context [%s]", context.c_str());
    // Using the getSection method from IniParser to fetch data.
    if (_validationFile->getSection(context, sectionData)) {
        return sectionData;
    }
    
    // If context isn't found, you could either return an empty map or handle the error appropriately.
    return std::map<std::string, std::string>();
}

bool Config::validateGlobalConfig() {
    for (std::map<std::string, std::vector<std::string> >::iterator it = _confData.begin(); it != _confData.end(); ++it) {
        try {
            for (std::vector<std::string>::const_iterator vec_it = it->second.begin(); vec_it != it->second.end(); ++vec_it) {
                log_message(Logger::DEBUG, "Vector content: %s", vec_it->c_str());
            }
            //new vector but removing the first element
            std::vector<std::string> newVec(it->second.begin() + 1, it->second.end());
            validateValue("global." + it->first, newVec, getFieldProperties("global." + it->first));
        } catch (std::exception& e) {
            log_message(Logger::ERROR, "Failed to validate global config key [%s]: %s", it->first.c_str(), e.what());
            return false;
        }
    }
    return true;
}

bool Config::validateVirtualServerConfig() {
    for (std::vector< ServerConfig >::iterator server_it = _servList.begin(); server_it != _servList.end(); ++server_it) {
        for (std::map<std::string, std::vector<std::string> >::iterator it = server_it->_servConfig.begin(); it != server_it->_servConfig.end(); ++it) {
            try {
                                //create new vector  but removing the first element
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
    return true; // All validations passed
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
    _validationFile->printAll();
    if (!validateGlobalConfig()) {
        return false;
    }
    if (!validateVirtualServerConfig()) {
        return false;
    }
    return true;
}




void Config::validateValue(const std::string& fullContext, const std::vector<std::string>& values, const std::map<std::string, std::string>& fieldProperties) {
    validationFactory&	factory = validationFactory::getInstance();
    ValidationStrategy*	strategy = NULL;
    
    try {
        // Extracting the validation type from the field properties
        std::map<std::string, std::string>::const_iterator strategyIt = fieldProperties.find("Validation");
        if (strategyIt == fieldProperties.end()) {
            log_message(Logger::ERROR, "Validation type not found for context [%s]", fullContext.c_str());
            return;
        }
                
        // Now getting the strategy using the extracted validation type
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
			return;
		}
    } catch (std::exception& e) {
        log_message(Logger::ERROR, "Error during context [%s], value [%s] validation: %s", fullContext.c_str(), values[0].c_str(), e.what());
        //delete strategy;
        throw; 
    }
    
    //delete strategy;
}
