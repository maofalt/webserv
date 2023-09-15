#ifndef ISVALIDSERVERNAME_HPP
#define ISVALIDSERVERNAME_HPP

#include "ValidationStrategy.hpp"

class isValidServerName : public ValidationStrategy {
public:
    virtual bool validate(const std::string& value, const std::map<std::string, std::string>& fieldProperties) const;
    virtual bool validate(const std::vector<std::string>& values, const std::map<std::string, std::string>& fieldProperties) const;
};


#endif // ISVALIDSERVERNAME_HPP