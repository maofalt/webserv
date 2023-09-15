// ValidationStrategy.hpp

#ifndef VALIDATIONSTRATEGY_HPP
#define VALIDATIONSTRATEGY_HPP

#include <string>
#include <map>
#include <set>
#include <vector> 
#include <sstream>
#include <algorithm>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "Utils.hpp"


typedef struct IniField {
    std::string Multiple;
    std::string Type;
    std::string Mandatory;
    std::string Default;
    std::string Min;
    std::string Max;
    std::string Validation;
    std::string Description;
} IniField;

// Base Strategy Interface
class ValidationStrategy {
public:
    virtual ~ValidationStrategy() {} // Virtual destructor to ensure derived classes get properly destroyed.
    
    // Main validati  on function that derived classes will implement.
    virtual bool validate(const std::string& value,             const std::map<std::string, std::string>& fieldProperties) const = 0;
    virtual bool validate(const std::vector<std::string>& values, const std::map<std::string, std::string>& fieldProperties) const = 0;
};
#endif // VALIDATIONSTRATEGY_HPP
