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


bool isPositiveInteger::validate(const std::vector<std::string> values, const std::map<std::string, std::string>& fieldProperties>) const {
    for (const std::vector<std::string>::const_iterator& it = values.begin(); it != values.end(); ++it) {
        if (!validate(*it, fieldProperties)) return false;
    }
    return true;
}

bool isPositiveInteger::validate(const std::string& value, const std::map<std::string, std::string>& fieldProperties) const {
        // Use the Default value if value is empty or wrong and a default is provided
    std::string actualValue = value;
    if(value.empty() && !fieldProperties.Default.empty()) {
        actualValue = fieldProperties.Default;
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
    ss >> value;
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
    if(!fieldProperties.Min.empty()) {
        int minValue = std::atoi(fieldProperties.Min.c_str());
        if(integerValue < minValue) {
            throw std::invalid_argument("Value is less than Min");
            return false; 
        }
    }

    if(!fieldProperties.Max.empty()) {
        int maxValue = std::atoi(fieldProperties.Max.c_str());
        if(integerValue > maxValue) {
            throw std::invalid_argument("Value is greater than Max");
            return false; 
        }
    }

    return true;
}


bool isValidServerName::validate(const std::vector<std::string>& values, const std::map<std::string, std::string>& fieldProperties) const {
    for (const std::vector<std::string>::const_iterator& it = values.begin(); it != values.end(); ++it) {
        if (!validate(*it, fieldProperties)) {
            return false; }
    }
    return true;
}

bool isValidServerName::validate(const std::string& value, const std::map<std::string, std::string>& fieldProperties) const {
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
    for (const std::vector<std::string>::const_iterator& it = values.begin(); it != values.end(); ++it) {
        if (!validate(*it, fieldProperties)) {
            return false; }
    }
    return true;
}

bool isValidDirectoryPath::validate(const std::string& value, const std::map<std::string>& fieldProperties) const {
    struct stat s;
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
    for (const std::vector<std::string>::const_iterator& it = values.begin(); it != values.end(); ++it) {
        if (!validate(*it, fieldProperties)) {
            return false; }
    }
    return true;
}

bool isValidPath::validate(const std::string& value, const std::map<std::string, std::string>& fieldProperties) const{
    struct stat s;
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

bool isValidMethod::validate(const std::map<std::string>& values, const std::map<std::string, std::string>& fieldProperties) const {
    for (const std::map<std::string>::const_iterator& it = values.begin(); it != values.end(); ++it) {
        if (!validate(*it, fieldProperties)) {
            return false; }
    }
}

bool isValidMethod::validate(const std::string& values, const std::map<std::string, std::string>& fieldProperties) const {
    std::string methods[] = {"GET", "POST", "DELETE"};
    for(int i = 0; i < 3; ++i) {
        if(value == methods[i]) {
            return true;
        }
    }
    throw std::invalid_argument("Method " + value + " is not valid");
    return false;
}

bool isValidRedirect::validate(const std::map<std::string>& value, const map<std::string, std::string>& fieldProperties) const {
    for (const std::map<std::string>::const_iterator& it = values.begin(); it != values.end(); ++it) {
        if (!validate(*it, fieldProperties)) {
            return false; }
    }
}

bool isValidRedirect::validate(const std::string& values, const std::map<std::string, std::string>& fieldProperties) const {
    
}

bool isValidOnOff::validate(const std::map<std::string>& values, const std::map<std::string, std::string>& fieldProperties) const{
    for (const std::map<std::string>::const_iterator& it = values.begin(); it != values.end(); ++it) {
        if (!validate(*it, fieldProperties)) {
            return false; }
    }
}

bool isValidOnOff::validate(const std::string& value, const std::map<std::string, std::string>& fieldProperties) const {
    
}

bool isValidCgiExtension::validate(const std::vector<std::string>& values, const std::map<std::string, std::string>& fieldProperties) const {
    for (const std::vector<std::string>::const_iterator& it = values.begin(); it != values.end(); ++it) {
        if (!validate(*it, fieldProperties)) {
            return false; }
    }
}

bool isValidCgiExtension::validate(const std::string& value, const std::map<std::string, std::string>& fieldProperties) const {
        
}

bool isValidFileUploadTypes::validate(const std::vector<std::string>& values, const std::map<std::string, std::string>& fieldProperties) const {
    for (const std::vecor<std::string>::const_iterator& it = values.begin(); it != values.end(); ++it) {
        if (!validate(*it, fieldProperties)) {
            return false; }
    }
}

bool isValidFileUploadTypes::validate(const std::vector<std::string>& values, const std::map<std::string, std::string>& fieldProperties) const {
        
}
