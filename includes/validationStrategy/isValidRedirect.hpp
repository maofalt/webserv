#ifndef ISVALIDREDIRECT_HPP
#define ISVALIDREDIRECT_HPP

#include "ValidationStrategy.hpp"

class isValidRedirect : public ValidationStrategy {
public:
    virtual bool validate(const std::string& value, const std::map<std::string, std::string>& fieldProperties) const;
    virtual bool validate(const std::vector<std::string>& values, const std::map<std::string, std::string>& fieldProperties) const;   
private:
    bool validateURL(const std::string& value) const;
    bool checkEmpty(const std::string& value) const;
    bool checkValidCharacters(const std::string& value) const;
    bool checkWildcardUsage(const std::string& value) const;
    bool checkDomainLength(const std::string& value) const;
    bool checkNumberOfDots(const std::string& value) const;
    bool checkConsecutiveDots(const std::string& value) const;
};

#endif // ISVALIDREDIRECT_HPP