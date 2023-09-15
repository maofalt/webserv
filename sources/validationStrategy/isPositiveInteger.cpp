/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   isPositiveInteger.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/02 21:19:07 by motero            #+#    #+#             */
/*   Updated: 2023/09/15 16:17:58 by motero           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "isPositiveInteger.hpp"


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
    std::map<std::string, std::string>::const_iterator minIt = fieldProperties.find("Min");
    if (minIt != fieldProperties.end()) {
        std::string min = minIt->second;
        int minValue = std::atoi(min.c_str());
        if(integerValue < minValue) {
            throw std::invalid_argument("Value is less than Min");
            return false; 
        }
    }

    std::map<std::string, std::string>::const_iterator itMax = fieldProperties.find("Max");
    if (itMax != fieldProperties.end()) {
        std::string max = itMax->second;
        if(!max.empty()) {
            int maxValue = std::atoi(max.c_str());
            if(integerValue > maxValue) {
                throw std::invalid_argument("Value is greater than Max");
                return false;  // Note: this line will not be reached due to the 'throw' statement above.
            }
        }
    }


    return true;
}
