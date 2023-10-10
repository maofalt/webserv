#ifndef CONFIGVALIDATOR_HPP
#define CONFIGVALIDATOR_HPP

#include <string>
#include <vector>
#include <map>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <stdint.h>
#include <iostream>
#include <sys/stat.h>
#include "Logger.hpp"
#include "IniParser.hpp"
#include "ServerConf.hpp"

class ConfigValidator {
private:
    IniParser&                                          _validationFile;
    std::map<std::string, std::vector<std::string> >&   _confData;
    std::vector<ServerConfig>&                          _servList;
    t_globalConfig                                      _globalConfig;

    typedef void (ConfigValidator::*setterFunc)(const std::string& );
    std::map<std::string, setterFunc>                   _setterMap;
    
   /* =============== Unnused Coplien =============== */

    ConfigValidator();

    /* =============== Utility Functions =============== */
    std::map<std::string, std::string> getFieldProperties(const std::string& context);
    void handleDuplicateValues(std::vector<std::string>& values, const std::map<std::string, std::string>& fieldProperties);
    void handleAllowedMultipleValues(std::vector<std::string>& values, const std::set<std::string>& uniqueValues);
    void handleNotAllowedMultipleValues(std::vector<std::string>& values, const std::vector<std::string>& subValues);
    void logValidatedConfigKey(const std::string& contextType, const std::string& configKey);
    void applyGlobalSettings(const std::string& configKey, const std::vector<std::string>& configValues);
    void logConfigKeyValidationError(const std::string& contextType, const std::string& configKey, const char* error);


    /* =============== Validation Core Functions =============== */
    void                validateValue(const std::string& fullContext, std::vector<std::string>& values, const std::map<std::string, std::string>& fieldProperties);
    ValidationStrategy* getValidationStrategy(const std::string& fullContext, const std::map<std::string, std::string>& fieldProperties);
    void                logValuesToValidate(const std::vector<std::string>& values);
    void                validateValuesUsingStrategy(const std::vector<std::string>& values, const std::map<std::string, std::string>& fieldProperties, ValidationStrategy* strategy, const std::string& fullContext);
    
    bool validateConfigData(std::map<std::string, std::vector<std::string> >& confData, const std::string& contextType);
    bool validateMandatoryKeys(std::map<std::string, std::vector<std::string> >& confData, const std::string& contextType);
    bool validateMandatoryKeys(std::map<std::string, std::string>& confData, const std::string& contextType);


    /* =============== Specific Configuration Validations =============== */
    bool validateGlobalConfig();
    bool validateVirtualServerConfig();
    bool validateLocationConfig(std::vector<location>& locations);

    /* =============== Setter Functions for globalconfig =============== */
    void setClientBodyLimit(const std::string& value);
    void setClientHeaderLimit(const std::string& value);
    void setTimeoutClient(const std::string& value);
    void setTimeoutCgi(const std::string& value);
    void setMaxConnections(const std::string& value);
    void setMaxRequests(const std::string& value);
    void setMaxRequestsPerIP(const std::string& value);

public:
    /* =============== Lifecycle Methods =============== */
    ConfigValidator(
    IniParser&                                          _validationFile, 
    std::map<std::string, std::vector<std::string> >&   _confData,
    std::vector<ServerConfig>&                          _servList
    );
    

    ~ConfigValidator();
    ConfigValidator &operator=(ConfigValidator const &rhs);

    /* =============== Accessor Methods =============== */
    const t_globalConfig&                                     getGlobalConfig() const;

    /* =============== Validation Methods =============== */
    bool validateConfig();
};


#endif // CONFIGVALIDATOR_HPP
