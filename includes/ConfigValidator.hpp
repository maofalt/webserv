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

    /* =============== Validation Core Functions =============== */
    void validateValue(const std::string& fullContext, std::vector<std::string>& values, const std::map<std::string, std::string>& fieldProperties);
    bool validateConfigData(std::map<std::string, std::vector<std::string> >& confData, const std::string& contextType);
    bool validateMandatoryKeys(const std::map<std::string, std::vector<std::string> >& confData, const std::string& contextType);
    bool validateMandatoryKeys(const std::map<std::string, std::string>& confData, const std::string& contextType);

    /* =============== Specific Configuration Validations =============== */
    bool validateGlobalConfig();
    bool validateVirtualServerConfig();
    bool validateLocationConfig(std::vector<location>& locations);

    /* =============== Setter Functions for globalconfig =============== */
    void setClientBodyLimit(const std::string& value);
    void setClientHeaderLimit(const std::string& value);
    void setTimeout(const std::string& value);
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
    
    /* =============== Accessor Methods =============== */
    const t_globalConfig&                                     getGlobalConfig() const;

    /* =============== Validation Methods =============== */
    bool validateConfig();
};


#endif // CONFIGVALIDATOR_HPP
