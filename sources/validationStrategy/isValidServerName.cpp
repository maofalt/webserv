/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   isValidServerName.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/02 21:19:07 by motero            #+#    #+#             */
/*   Updated: 2023/09/15 15:26:45 by motero           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ValidationStrategy.hpp"


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
            throw std::invalid_argument(std::string("ServerName ") + value + std::string(" contains invalid characters: ") + *it);
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
