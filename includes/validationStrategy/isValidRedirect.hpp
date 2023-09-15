#ifndef ISVALIDREDIRECT_HPP
#define ISVALIDREDIRECT_HPP

#include "ValidationStrategy.hpp"

class isValidRedirect : public ValidationStrategy {
public:
    virtual bool validate(const std::string& value, const std::map<std::string, std::string>& fieldProperties) const;
    virtual bool validate(const std::vector<std::string>& values, const std::map<std::string, std::string>& fieldProperties) const;   
};

#endif // ISVALIDREDIRECT_HPP