/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: znogueir <znogueir@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/24 21:55:01 by rgarrigo          #+#    #+#             */
/*   Updated: 2023/09/01 18:16:12 by znogueir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "validationFactory.hpp"
#include "ValidationStrategy.hpp"  // Assuming this is the file where your ValidationStrategy and derived classes are defined.

// Private method to register the available strategies.
void validationFactory::registerStrategies() {
    try {
        strategyMap["isPositiveInteger"] = new isPositiveInteger();
        strategyMap["isValidServerName"] = new isValidServerName();
        strategyMap["isValidDirectoryPath"] = new isValidDirectoryPath();
        strategyMap["isValidPath"] = new isValidPath();
        strategyMap["isValidMethod"] = new isValidMethod();
        strategyMap["isValidRedirect"] = new isValidRedirect();
        strategyMap["isValidOnOff"] = new isValidOnOff();
        strategyMap["isValidCgiExtension"] = new isValidCgiExtension();
        strategyMap["isValidFileUploadTypes"] = new isValidFileUploadTypes();
    } catch (std::bad_alloc& ba) {
        cleanupStrategies();
        throw("Memory allocation failed while registering strategies: " + std::string(ba.what()));
    }
}

// Singleton access method using Meyer's Singleton
validationFactory& validationFactory::getInstance() {
    static validationFactory instance; // This is the Meyer's Singleton part.
    return instance;
}

ValidationStrategy* validationFactory::getStrategy(const std::string& validationType) {
    // Fetch the corresponding validation strategy from the map.
    std::map<std::string, ValidationStrategy*>::iterator it = strategyMap.find(validationType);
    if (it != strategyMap.end()) {
        return it->second;
    } else {
        log_message(Logger::WARN, "No such strategy defined: %s", validationType.c_str());
        return NULL;  // No such strategy defined.
    }
}

void validationFactory::cleanupStrategies() {
    std::map<std::string, ValidationStrategy*>::iterator it;
    for (it = strategyMap.begin(); it != strategyMap.end(); ++it) {
        delete it->second;
        it->second = NULL; // C++98 equivalent of nullptr
    }
    strategyMap.clear(); // Ensure the map is empty after cleanup.
}


validationFactory::validationFactory() {
    registerStrategies(); // Populate the map with the available strategies.
}

validationFactory::~validationFactory() {
    // Cleanup dynamically allocated strategies.
    for (std::map<std::string, ValidationStrategy*>::iterator it = strategyMap.begin(); it != strategyMap.end(); ++it) {
        delete it->second;
    }
}
