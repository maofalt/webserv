/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   validationFactory.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/24 21:55:01 by rgarrigo          #+#    #+#             */
/*   Updated: 2023/09/13 17:01:34 by motero           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ValidationFactory.hpp"
#include "ValidationStrategy.hpp"  // Assuming this is the file where your ValidationStrategy and derived classes are defined.

// Private method to register the available strategies.
void ValidationFactory::registerStrategies() {
    try {
        strategyMap["isPositiveInteger"] = new isPositiveInteger();
        strategyMap["isValidServerName"] = new isValidServerName();
        strategyMap["isValidDirectoryPath"] = new isValidDirectoryPath();
        strategyMap["isValidLocPath"] = new isValidLocPath();
        strategyMap["isValidPath"] = new isValidPath();
        strategyMap["isValidMethod"] = new isValidMethod();
        strategyMap["isValidRedirect"] = new isValidRedirect();
        strategyMap["isValidOnOff"] = new isValidOnOff();
        strategyMap["isValidCgiExtension"] = new isValidCgiExtension();
        strategyMap["isValidFileUploadTypes"] = new isValidFileUploadTypes();
        strategyMap["isValidAllow"] = new isValidAllow();
    } catch (const std::exception& e) {
        cleanupStrategies();
        throw("Memory allocation failed while registering strategies: " + std::string(e.what()));
    }
}

// Singleton access method using Meyer's Singleton
ValidationFactory& ValidationFactory::getInstance() {
    static ValidationFactory instance; // This is the Meyer's Singleton part.
    return instance;
}

ValidationStrategy* ValidationFactory::getStrategy(const std::string& validationType) {
    // Fetch the corresponding validation strategy from the map.
    std::map<std::string, ValidationStrategy*>::iterator it = strategyMap.find(validationType);
    if (it != strategyMap.end()) {
        return it->second;
    } else {
        log_message(Logger::WARN, "No such context defined: %s", validationType.c_str());
        return NULL;  // No such strategy defined.
    }
}

void ValidationFactory::cleanupStrategies() {
    log_message(Logger::DEBUG, "Cleaning up strategies because they failed to register.");
    std::map<std::string, ValidationStrategy*>::iterator it;
    for (it = strategyMap.begin(); it != strategyMap.end(); ++it) {
        delete it->second;
        it->second = NULL;
    }
    strategyMap.clear(); // Ensure the map is empty after cleanup.
}


ValidationFactory::ValidationFactory() {
    registerStrategies(); // Populate the map with the available strategies.
}

ValidationFactory::~ValidationFactory() {
    // Cleanup dynamically allocated strategies.
    for (std::map<std::string, ValidationStrategy*>::iterator it = strategyMap.begin(); it != strategyMap.end(); ++it) {
        delete it->second;
    }
}
