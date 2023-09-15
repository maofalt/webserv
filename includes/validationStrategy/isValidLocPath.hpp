#ifndef ISVALIDLOCPATH_HPP
#define ISVALIDLOCPATH_HPP

#include "ValidationStrategy.hpp"

class isValidLocPath : public ValidationStrategy {
public:
    virtual bool validate(const std::string& value, const std::map<std::string, std::string>& fieldProperties) const;
    virtual bool validate(const std::vector<std::string>& values, const std::map<std::string, std::string>& fieldProperties) const;   
};


#endif // ISVALIDLOCPATH_HPP