#ifndef ISVALIDSERVERNAME_HPP
#define ISVALIDSERVERNAME_HPP

#include "ValidationStrategy.hpp"

class isValidServerName : public ValidationStrategy {
public:
    virtual bool validate(const std::string& value, const std::map<std::string, std::string>& fieldProperties) const;
    virtual bool validate(const std::vector<std::string>& values, const std::map<std::string, std::string>& fieldProperties) const;
private:
    bool checkNonEmptiness(const std::string& value) const;
    bool checkValidCharacters(const std::string& value) const;
    bool checkWildcardUsage(const std::string& value) const;
    bool checkDomainLength(const std::string& value) const;
    bool checkSegmentLength(const std::string& value) const;
    bool checkNumberOfDots(const std::string& value) const;
    bool checkConsecutiveDots(const std::string& value) const;
};


#endif // ISVALIDSERVERNAME_HPP