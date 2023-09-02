#ifndef INI_PARSER_HPP
#define INI_PARSER_HPP

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <set>
#include <cctype>     // for std::isspace
#include <algorithm>  // for std::find_if
#include <functional> // for std::not1 and std::ptr_fun
#include "Utils.hpp"


class IniParser {
private:
    std::map<std::string, std::map<std::string, std::string> >  data;
    static std::set<std::string>                                _validKeys;
    bool                                                        _errorInSection;
 

    void                                                        trim(std::string& str);
    bool                                                        isCommentOrEmpty(const std::string& line) const;
    void                                                        handleSection(const std::string& line, std::string& currentSection);

//Validating parsing methods    
    void                                                        handleKeyValuePair(const std::string& line, const std::string& currentSection);
    bool                                                        isDuplicateKey(const std::string& key, const std::string& currentSection);
    bool                                                        isValidKey(const std::string& key);
    bool                                                        isValidValueForKey(const std::string& key, const std::string& value);
    void                                                        logInvalidKeyValuePair(const std::string& line, const std::string& currentSection);
    void                                                        logDuplicateKey(const std::string& key, const std::string& currentSection);
    void                                                        logInvalidKey(const std::string& key, const std::string& currentSection);
    void                                                        logInvalidValueForKey(const std::string& value, const std::string& key, const std::string& currentSection);
    void                                                        initializeValidKeys();

//helper methods for readability

    void                                                        checkFileStatus(std::ifstream& file) const; 
    bool                                                        isNewSection(const std::string& line) const;
    void                                                        finalizePreviousSection(std::string& previousSection, std::string& currentSection);
    void                                                        checkAndEraseEmptySection(const std::string& section);
    void                                                        finalizeLoading(const std::string& previousSection, const std::string& currentSection);
//field validation methods to place in another header
    typedef bool                                                (*ValidationFunction)(const std::string&);
    static std::map<std::string, ValidationFunction>            _validationFunctions;
    void                                                        initializeValidationFunctions();
    static bool                                                 isTypeValid(const std::string& value);
    static bool                                                 isInteger(const std::string& value);
    static bool                                                 isMinValid(const std::string& value);
    static bool                                                 isMaxValid(const std::string& value);
    static bool                                                 isMandatoryValid(const std::string& value);
    static bool                                                 isDefaultValid(const std::string& value);
    static bool                                                 isValidationStrategyValid(const std::string& value);
//type validation methods no plac in another header
    static std::string                                           _currentType;
    static std::map<std::string, ValidationFunction>             _validTypes;
    void                                                        initializeValidTypes();                       
    static bool                                                 isValidInteger(const std::string& value);
    static bool                                                 isValidString(const std::string& value);
    static bool                                                 isValidBoolean(const std::string& value);
    static bool                                                 isValidIpv4(const std::string& value);
    static bool                                                 isValidPort(const std::string& value);
public:
    IniParser();

    int     loadConfig(const std::string& filename);
    

    // Function to fetch value given a section and key.
    bool getValue(const std::string& section, const std::string& key, std::string& value) const;

    // Function to fetch all keys and values in a section
    bool getSection(const std::string& section, std::map<std::string, std::string>& sectionData) const;

    // Debug function to print all sections, keys and values
    void printAll() const;
};

#endif // INI_PARSER_H
