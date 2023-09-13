#ifndef VALIDATION_FACTORY_H
#define VALIDATION_FACTORY_H

#include <string>
#include <map>
#include "ValidationStrategy.hpp"

class ValidationFactory {
public:
    // Singleton access method
    static ValidationFactory& getInstance();

    // Fetch the corresponding validation strategy. Returns null if no such strategy is defined.
    ValidationStrategy* getStrategy(const std::string& validationType);

    // Prevent copying and assignment

private:
    ValidationFactory();
    ~ValidationFactory();

    ValidationFactory(const ValidationFactory&);
    ValidationFactory& operator=(const ValidationFactory&);

    // Populates the map with the available strategies.
    void registerStrategies();
    void cleanupStrategies();

    // Map linking validation type strings to their corresponding strategies.
    std::map<std::string, ValidationStrategy*> strategyMap;
};

#endif // VALIDATION_FACTORY_H
