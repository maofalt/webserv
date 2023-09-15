#ifndef ISPOSITIVEINTEGER_HPP
#define ISPOSITIVEINTEGER_HPP

#include "ValidationStrategy.hpp"

class isPositiveInteger : public ValidationStrategy {
public:
    virtual bool validate(const std::string& value, const std::map<std::string, std::string>& fieldProperties) const;
    virtual bool validate(const std::vector<std::string>& values, const std::map<std::string, std::string>& fieldProperties) const;
};

#endif // ISPOSITIVEINTEGER_HPP
