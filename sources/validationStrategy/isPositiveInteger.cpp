/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   isPositiveInteger.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/02 21:19:07 by motero            #+#    #+#             */
/*   Updated: 2023/09/15 16:40:05 by motero           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "isPositiveInteger.hpp"


bool isPositiveInteger::validate(const std::vector<std::string>& values, const std::map<std::string, std::string>& fieldProperties) const {
    for (std::vector<std::string>::const_iterator it = values.begin(); it != values.end(); ++it) {
        if (!validate(*it, fieldProperties)) return false;
    }
    return true;
}

std::string isPositiveInteger::getDefaultValue(const std::map<std::string, std::string>& fieldProperties) const {
    std::map<std::string, std::string>::const_iterator it = fieldProperties.find("Default");
    return (it != fieldProperties.end()) ? it->second : std::string();
}

bool isPositiveInteger::isInteger(const std::string& value) const {
    std::string::const_iterator it = value.begin();
    while (it != value.end() && std::isdigit(*it)) ++it;
    return value.empty() || it == value.end();
}

int isPositiveInteger::convertToInt(const std::string& value) const {
    std::stringstream ss(value);
    int integerValue;
    ss >> integerValue;

    if (ss.fail()) {
        throw std::invalid_argument("Value overflows or underflows");
    }

    return integerValue;
}

bool isPositiveInteger::isInValidRange(int value, const std::map<std::string, std::string>& fieldProperties) const {
    std::map<std::string, std::string>::const_iterator itMin = fieldProperties.find("Min");
    if (itMin != fieldProperties.end() && !itMin->second.empty()) {
        int minValue = std::atoi(itMin->second.c_str());
        if (value < minValue) {
            throw std::invalid_argument("Value is less than Min");
        }
    }

    std::map<std::string, std::string>::const_iterator itMax = fieldProperties.find("Max");
    if (itMax != fieldProperties.end() && !itMax->second.empty()) {
        int maxValue = std::atoi(itMax->second.c_str());
        if (value > maxValue) {
            throw std::invalid_argument("Value is greater than Max");
        }
    }

    return true;
}

bool isPositiveInteger::validate(const std::string& value, const std::map<std::string, std::string>& fieldProperties) const {
    std::string actualValue = value;
    std::string defaultValue = getDefaultValue(fieldProperties);

    if (value.empty() && !defaultValue.empty()) {
        actualValue = defaultValue;
    }

    if (!isInteger(actualValue)) {
        throw std::invalid_argument("Value is not an integer");
    }

    int integerValue = convertToInt(actualValue);

    if (integerValue < 0) {
        throw std::invalid_argument("Value is not positive");
    }

    return isInValidRange(integerValue, fieldProperties);
}

