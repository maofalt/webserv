#ifndef ISVALIDMETHOD_HPP
#define ISVALIDMETHOD_HPP

#include "ValidationStrategy.hpp"

class isValidMethod : public ValidationStrategy {
public:
    virtual bool validate(const std::string& value, const std::map<std::string, std::string>& fieldProperties) const;
    virtual bool validate(const std::vector<std::string>& values, const std::map<std::string, std::string>& fieldProperties) const;   
};


#endif // ISVALIDMETHOD_HPP