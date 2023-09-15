#ifndef ISVALIDONOFF_HPP
#define ISVALIDONOFF_HPP

#include "ValidationStrategy.hpp"

class isValidOnOff : public ValidationStrategy {
public:
    virtual bool validate(const std::string& value, const std::map<std::string, std::string>& fieldProperties) const;
    virtual bool validate(const std::vector<std::string>& values, const std::map<std::string, std::string>& fieldProperties) const;   
};
#endif // ISVALIDONOFF_HPP