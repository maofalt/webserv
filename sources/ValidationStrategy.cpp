/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ValidationStrategy.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/02 21:19:07 by motero            #+#    #+#             */
/*   Updated: 2023/09/02 22:51:00 by motero           ###   ########.fr       */
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


