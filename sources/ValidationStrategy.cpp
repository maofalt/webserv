/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ValidationStrategy.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/02 21:19:07 by motero            #+#    #+#             */
/*   Updated: 2023/09/02 23:40:24 by motero           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ValidationStrategy.hpp"


// Split function to break down comma-separated values
std::vector<std::string> split(const std::string &s, char delimiter) {
    
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}


bool isPositiveInteger::validate(const std::vector<std::string>& values, const std::map<std::string, std::string>& fieldProperties) const {
    for (std::vector<std::string>::const_iterator it = values.begin(); it != values.end(); ++it) {
        if (!validate(*it, fieldProperties)) return false;
    }
    return true;
}

bool isPositiveInteger::validate(const std::string& value, const std::map<std::string, std::string>& fieldProperties) const {
        // Use the Default value if value is empty or wrong and a default is provided
    std::string actualValue = value;
    std::string defaultValue = fieldProperties.find("Default")->second;
    if(value.empty() && !defaultValue.empty()) {
        actualValue = defaultValue;
    }

    // Check if actualValue is an integer
    std::string::const_iterator it = actualValue.begin();
    while (it != actualValue.end() && std::isdigit(*it)) ++it;
    if (!actualValue.empty() && it != actualValue.end()) {
        throw std::invalid_argument("Value is not an integer");
        return false; 
    }

    //We check if is an integer by converting it to int
    std::stringstream ss(value);
    int integerValue;
    ss >> integerValue;
    if (ss.fail()) {
        ss.clear();
        throw std::invalid_argument("Value overflows or underflows");
        return false;
    }

    if (integerValue < 0) {
        throw std::invalid_argument("Value is not positive");
        return false;
    }

    // Validate Min and Max
    std::string min = fieldProperties.find("Min")->second;
    if(!min.empty()) {
        int minValue = std::atoi(min.c_str());
        if(integerValue < minValue) {
            throw std::invalid_argument("Value is less than Min");
            return false; 
        }
    }

    std::string max = fieldProperties.find("Max")->second;
    if(!max.empty()) {
        int maxValue = std::atoi(max.c_str());
        if(integerValue > maxValue) {
            throw std::invalid_argument("Value is greater than Max");
            return false; 
        }
    }

    return true;
}


bool isValidServerName::validate(const std::vector<std::string>& values, const std::map<std::string, std::string>& fieldProperties) const {
    for (std::vector<std::string>::const_iterator it = values.begin(); it != values.end(); ++it) {
        if (!validate(*it, fieldProperties)) {
            return false; }
    }
    return true;
}

bool isValidServerName::validate(const std::string& value, const std::map<std::string, std::string>& fieldProperties) const {
    
    (void)fieldProperties;
    // 1. Non-emptiness
    if (value.empty()) {
        throw std::invalid_argument("ServerName is empty");
        return false;
    }

    // 2. Valid Characters
    for (std::string::const_iterator it = value.begin(); it != value.end(); ++it) {
        if (!std::isalnum(*it) && *it != '-' && *it != '.' && *it != '*') {
            throw std::invalid_argument("ServerName contains invalid characters: " + *it);
            return false;
        }
    }

    // 3. Wildcard Usage
    size_t wildcard_pos = value.find('*');
    if (wildcard_pos != std::string::npos && wildcard_pos != 0) {
        throw std::invalid_argument("ServerName contains wildcard in invalid position");
        return false;
    }
    if (value.find('*', wildcard_pos + 1) != std::string::npos) {
        throw std::invalid_argument("ServerName contains multiple wildcards");
        return false;
    }

    // 4. Domain Length
    if (value.length() > 253) {
        throw std::invalid_argument("ServerName is too long [253 characters max]");
        return false;
    }

    // Using a stringstream to split the string by dots
    std::stringstream ss(value);
    std::string segment;
    while (std::getline(ss, segment, '.')) {
        if (segment.length() > 63) {
            throw std::invalid_argument("ServerName contains a domain that is too long [63 characters max]:" + segment);
            return false;
        }
    }

    // 5. Number of Dots
    if (std::count(value.begin(), value.end(), '.') < 1) {
        throw std::invalid_argument("ServerName does not contain a domain");
        return false;
    }

    // 6. No Consecutive Dots
    if (value.find("..") != std::string::npos) {
        throw std::invalid_argument("ServerName contains consecutive dots");
        return false;
    }

    return true;
}

bool isValidDirectoryPath::validate(const std::vector<std::string>& values, const std::map<std::string, std::string>& fieldProperties) const {
    for (std::vector<std::string>::const_iterator it = values.begin(); it != values.end(); ++it) {
        if (!validate(*it, fieldProperties)) {
            return false; }
    }
    return true;
}

bool isValidDirectoryPath::validate(const std::string& value, const std::map<std::string, std::string>& fieldProperties) const {
    struct stat s;
    (void)fieldProperties;
    if(stat(value.c_str(), &s) == 0) {
        if(s.st_mode & S_IFDIR) {
            // It's a directory
            return true;
        } else if(s.st_mode & S_IFREG) {
            throw std::invalid_argument("Path is a file, not a directory");
            return false;
        } else {
            throw std::invalid_argument("Path is not a file or directory (i think....)");
            return false;
        }
    } else {
        throw std::invalid_argument("Directory does not exist or don't have permissions");
        // Error could be due to many reasons like file/directory doesn't exist or isn't accessible.
        return false;
    }
}

bool isValidPath::validate(const std::vector<std::string>& values, const std::map<std::string, std::string>& fieldProperties) const {
    for (std::vector<std::string>::const_iterator it = values.begin(); it != values.end(); ++it) {
        if (!validate(*it, fieldProperties)) {
            return false; }
    }
    return true;
}

bool isValidPath::validate(const std::string& value, const std::map<std::string, std::string>& fieldProperties) const{
    struct stat s;
    (void)fieldProperties;
    if(stat(value.c_str(), &s) == 0) {
        if(s.st_mode & S_IFREG) {
            return true;
        } else {
            throw std::invalid_argument("Path is not a file");
            return false;
        }
    } else {
        throw std::invalid_argument("File does not exist or don't have permissions. You stink");
        return false;
    }   
}

bool isValidMethod::validate(const std::vector<std::string>& values, const std::map<std::string, std::string>& fieldProperties) const {
    for (std::vector<std::string>::const_iterator it = values.begin(); it != values.end(); ++it) {
        if (!validate(*it, fieldProperties)) {
            return false; }
    }
    return true;
}

bool isValidMethod::validate(const std::string& value, const std::map<std::string, std::string>& fieldProperties) const {
    std::string methods[] = {"GET", "POST", "DELETE"};
    (void)fieldProperties;
    for(int i = 0; i < 3; ++i) {
        if(value == methods[i]) {
            return true;
        }
    }
    throw std::invalid_argument("Method " + value + " is not valid");
    return false;
}

bool isValidRedirect::validate(const std::vector<std::string>& values, const std::map<std::string, std::string>& fieldProperties) const {
    (void)fieldProperties;
    // Check if the vector has only 2 strings
    if (values.size() != 2) {
        throw std::invalid_argument("Redirection does not contain exactly two values.");
        return false;
    }

    // Check for valid redirection codes
    std::set<std::string> validCodes;
    validCodes.insert("301");
    validCodes.insert("302");
    validCodes.insert("303");
    validCodes.insert("307");
    validCodes.insert("308");
    if (validCodes.find(values[0]) == validCodes.end()) {
        throw std::invalid_argument("Invalid redirection type: " + values[0]);
        return false;
    }
    // Check the URL (using a modified version of your provided validation)
    const std::string& value = values[1];

    // 1. Check if Empty
    if (value.empty()) {
        throw std::invalid_argument("URL is empty");
        return false;
    }

    // 2. Valid Characters
    for (std::string::const_iterator it = value.begin(); it != value.end(); ++it) {
        if (!std::isalnum(*it) && *it != '-' && *it != '.' && *it != '*' && *it != '/' && *it != ':' && *it != '_') {
            throw std::invalid_argument("URL contains invalid characters: " + *it);
            return false;
        }
    }

    // 3. Wildcard Usage (wildcards in URLs are more unusual, so let's not allow them)
    if (value.find('*') != std::string::npos) {
        throw std::invalid_argument("URL contains a wildcard");
        return false;
    }

    // 4. Domain Length
    if (value.length() > 2048) { // Most browsers support URLs up to 2048 characters
        throw std::invalid_argument("URL is too long [2048 characters max]");
        return false;
    }

    // 5. Number of Dots
    if (std::count(value.begin(), value.end(), '.') < 1) {
        throw std::invalid_argument("URL does not contain a domain");
        return false;
    }

    // 6. No Consecutive Dots
    if (value.find("..") != std::string::npos) {
        throw std::invalid_argument("URL contains consecutive dots");
        return false;
    }

    return true;
}

bool isValidRedirect::validate(const std::string& value, const std::map<std::string, std::string>& fieldProperties) const {
    (void)value;
    (void)fieldProperties;
    return true;
}

bool isValidOnOff::validate(const std::vector<std::string>& values, const std::map<std::string, std::string>& fieldProperties) const{
    for (std::vector<std::string>::const_iterator it = values.begin(); it != values.end(); ++it) {
        if (!validate(*it, fieldProperties)) {
            return false; }
    }
    return true;
}

bool isValidOnOff::validate(const std::string& value, const std::map<std::string, std::string>& fieldProperties) const {
    (void)fieldProperties;
    
    std::vector<std::string> validValues;
    validValues.push_back("on");
    validValues.push_back("off");
    if (std::find(validValues.begin(), validValues.end(), value) == validValues.end()) {
        throw std::invalid_argument("Invalid value for OnOff: " + value);
        return false;
    }

    return true;
}

bool isValidCgiExtension::validate(const std::vector<std::string>& values, const std::map<std::string, std::string>& fieldProperties) const {
    for (std::vector<std::string>::const_iterator it = values.begin(); it != values.end(); ++it) {
        if (!validate(*it, fieldProperties)) {
            return false; }
    }
    return true;
}

bool isValidCgiExtension::validate(const std::string& value, const std::map<std::string, std::string>& fieldProperties) const {
    (void)fieldProperties; // Unused

    // 1. Check for known extensions
    if (value.find("/php-cgi") == std::string::npos && value.find("/python-cgi") == std::string::npos) {
        throw std::invalid_argument("Invalid CGI extension in path: " + value);
        return false;
    }

    // 2. Ensure the path exists
    struct stat buffer;
    if (stat(value.c_str(), &buffer) != 0) {
        throw std::invalid_argument("Path does not exist: " + value);
        return false;
    }

    // 3. Ensure the path is a file
    if (S_ISDIR(buffer.st_mode)) {
        throw std::invalid_argument("Path is a directory, not a file: " + value);
        return false;
    }

    // 4. Check if the current user has execute rights on the file
    if (access(value.c_str(), X_OK) != 0) {
        throw std::invalid_argument("No execute rights on the file: " + value);
        return false;
    }

    return true;
}

bool isValidFileUploadTypes::validate(const std::vector<std::string>& values, const std::map<std::string, std::string>& fieldProperties) const {
    for (std::vector<std::string>::const_iterator it = values.begin(); it != values.end(); ++it) {
        if (!validate(*it, fieldProperties)) {
            return false; }
    }
    return true;
}

bool isValidFileUploadTypes::validate(const std::string& value, const std::map<std::string, std::string>& fieldProperties) const {
    
    (void)fieldProperties; // Unused
    // Extract valid types from fieldProperties or use a hardcoded set.
    std::set<std::string> validTypes;
    validTypes.insert("jpg");
    validTypes.insert("jpeg");
    validTypes.insert("png");
    validTypes.insert("gif");
    // Split the 'value' by commas or spaces, if you expect multiple values.
    std::vector<std::string> givenTypes;
    std::stringstream ss(value);
    std::string item;
    while (std::getline(ss, item, ',')) { // Assuming ',' as delimiter.
        givenTypes.push_back(item);
    }
    // Check each given type against validTypes.
    for (std::vector<std::string>::iterator type = givenTypes.begin(); type != givenTypes.end(); ++type) {
        if (validTypes.find(*type) == validTypes.end()) {
            throw std::invalid_argument("Invalid file type: " + *type);
            return false; // Found an invalid type.
        }
    }
    return true; // All given types are valid.

}
