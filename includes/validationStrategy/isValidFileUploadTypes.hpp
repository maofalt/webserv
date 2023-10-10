#ifndef ISVALIDFILEUPLOADTYPES_HPP
#define ISVALIDFILEUPLOADTYPES_HPP

#include "ValidationStrategy.hpp"

class isValidFileUploadTypes : public ValidationStrategy {
public:
    virtual bool validate(const std::string& value, const std::map<std::string, std::string>& fieldProperties) const;
    virtual bool validate(const std::vector<std::string>& values, const std::map<std::string, std::string>& fieldProperties) const;   
private:
    std::set<std::string>       getValidTypes() const;
    std::vector<std::string>    splitValue(const std::string& value) const;
    bool                        checkAgainstValidTypes(const std::vector<std::string>& givenTypes, const std::set<std::string>& validTypes) const;
};
#endif // ISVALIDFILEUPLOADTYPES_HPP