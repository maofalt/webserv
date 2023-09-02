// ValidationStrategy.hpp

#ifndef VALIDATIONSTRATEGY_HPP
#define VALIDATIONSTRATEGY_HPP

#include <string>
#include <map>

// Base Strategy Interface
class ValidationStrategy {
public:
    virtual ~ValidationStrategy() {} // Virtual destructor to ensure derived classes get properly destroyed.
    
    // Main validation function that derived classes will implement.
    virtual bool validate(const std::string& value, const std::map<std::string, std::string>& fieldProperties) const = 0;
};


class IsInteger : public ValidationStrategy {
public:
    virtual bool validate(const std::string& value, const std::map<std::string, std::string>& fieldProperties) const;
};

class IsPositiveInteger : public ValidationStrategy {
public:
    virtual bool validate(const std::string& value, const std::map<std::string, std::string>& fieldProperties) const;
};

class IsValidHostname : public ValidationStrategy {
public:
    virtual bool validate(const std::string& value, const std::map<std::string, std::string>& fieldProperties) const;
};

class UniqueList : public ValidationStrategy {
public:
    virtual bool validate(const std::string& value, const std::map<std::string, std::string>& fieldProperties) const;
};

class PathExistenceMap : public ValidationStrategy {
public:
    virtual bool validate(const std::string& value, const std::map<std::string, std::string>& fieldProperties) const;
};

class IsValidPatternList : public ValidationStrategy {
public:
    virtual bool validate(const std::string& value, const std::map<std::string, std::string>& fieldProperties) const;
};

class ListContainsValidMethods : public ValidationStrategy {
public:
    virtual bool validate(const std::string& value, const std::map<std::string, std::string>& fieldProperties) const;
};

class IsValidURLList : public ValidationStrategy {
public:
    virtual bool validate(const std::string& value, const std::map<std::string, std::string>& fieldProperties) const;
};

class PathExistenceList : public ValidationStrategy {
public:
    virtual bool validate(const std::string& value, const std::map<std::string, std::string>& fieldProperties) const;
};

class IsValidOnOffSettingList : public ValidationStrategy {
public:
    virtual bool validate(const std::string& value, const std::map<std::string, std::string>& fieldProperties) const;
};

class FileExistenceList : public ValidationStrategy {
public:
    virtual bool validate(const std::string& value, const std::map<std::string, std::string>& fieldProperties) const;
};

class IsValidFileTypesList : public ValidationStrategy {
public:
    virtual bool validate(const std::string& value, const std::map<std::string, std::string>& fieldProperties) const;
};

#endif // VALIDATIONSTRATEGY_HPP
