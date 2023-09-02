/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ValidationStrategy.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/02 21:19:07 by motero            #+#    #+#             */
/*   Updated: 2023/09/02 22:00:32 by motero           ###   ########.fr       */
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

bool validate(const std::vector<std::string>& value, const std::map<std::string, std::string>& fieldProperties) {
    for (const std::vector<std::string>::const_iterator& it = value.begin(); it != value.end(); ++it) {
        if (!validate(*it, fieldProperties)) return false;
    }
    return true;
}

bool IsInteger::validate(const std::string& value, const IniField& fieldProperties) const {

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


bool IsPositiveInteger::validate(const std::string& value, const std::map<std::string, std::string>& fieldProperties) const {
    for(char c : value) {
        if(!isdigit(c)) return false;
    }
    int val = std::atoi(value.c_str());
    return val > 0;
}

bool IsValidHostname::validate(const std::string& value, const std::map<std::string, std::string>& fieldProperties) const {
    // Placeholder logic: You'll need a proper hostname validation here
    return value.length() > 0 && value.find(' ') == std::string::npos;
}

bool UniqueList::validate(const std::string& value, const std::map<std::string, std::string>& fieldProperties) const {
    std::vector<std::string> items = split(value, ',');
    std::set<std::string> uniqueItems(items.begin(), items.end());
    return items.size() == uniqueItems.size();
}

bool PathExistenceMap::validate(const std::string& value, const std::map<std::string, std::string>& fieldProperties) const {
    // Placeholder logic: For each path in the map, ensure it exists on the file system
    return true; // Replace with actual logic
}

bool IsValidPatternList::validate(const std::string& value, const std::map<std::string, std::string>& fieldProperties) const {
    // Placeholder logic: Check if each pattern in list is valid
    return true;
}

bool ListContainsValidMethods::validate(const std::string& value, const std::map<std::string, std::string>& fieldProperties) const {
    // Placeholder logic: Check if each method in the list is a recognized method
    return true;
}

bool IsValidURLList::validate(const std::string& value, const std::map<std::string, std::string>& fieldProperties) const {
    // Placeholder logic: For each URL, ensure it's a valid URL
    return true;
}

bool PathExistenceList::validate(const std::string& value, const std::map<std::string, std::string>& fieldProperties) const {
    // Placeholder logic: For each path in the list, ensure it exists
    return true;
}

bool IsValidOnOffSettingList::validate(const std::string& value, const std::map<std::string, std::string>& fieldProperties) const {
    std::vector<std::string> items = split(value, ',');
    for(const std::string& item : items) {
        if(item != "on" && item != "off") return false;
    }
    return true;
}

bool FileExistenceList::validate(const std::string& value, const std::map<std::string, std::string>& fieldProperties) const {
    // Placeholder logic: For each file in the list, ensure it exists
    return true;
}

bool IsValidFileTypesList::validate(const std::string& value, const std::map<std::string, std::string>& fieldProperties) const {
    // Placeholder logic: Check if each file type is valid
    return true;
}
