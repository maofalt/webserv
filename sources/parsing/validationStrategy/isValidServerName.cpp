/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   isValidServerName.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/02 21:19:07 by motero            #+#    #+#             */
/*   Updated: 2023/09/15 16:55:25 by motero           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "isValidServerName.hpp"


bool isValidServerName::validate(const std::vector<std::string>& values, const std::map<std::string, std::string>& fieldProperties) const {
    for (std::vector<std::string>::const_iterator it = values.begin(); it != values.end(); ++it) {
        if (!validate(*it, fieldProperties)) {
            return false; }
    }
    return true;
}
bool isValidServerName::checkNonEmptiness(const std::string& value) const {
    if (value.empty()) {
        throw std::invalid_argument("ServerName is empty");
        return false;
    }
    return true;
}

bool isValidServerName::checkValidCharacters(const std::string& value) const {
    for (std::string::const_iterator it = value.begin(); it != value.end(); ++it) {
        if (!std::isalnum(*it) && *it != '-' && *it != '.' && *it != '*') {
            throw std::invalid_argument(std::string("ServerName ") + value + std::string(" contains invalid characters: ") + *it);
            return false;
        }
    }
    return true;
}

bool isValidServerName::checkWildcardUsage(const std::string& value) const {
    size_t wildcard_pos = value.find('*');
    if (wildcard_pos != std::string::npos && wildcard_pos != 0) {
        throw std::invalid_argument("ServerName contains wildcard in invalid position");
        return false;
    }
    if (value.find('*', wildcard_pos + 1) != std::string::npos) {
        throw std::invalid_argument("ServerName contains multiple wildcards");
        return false;
    }
    return true;
}

bool isValidServerName::checkDomainLength(const std::string& value) const {
    if (value.length() > 253) {
        throw std::invalid_argument("ServerName is too long [253 characters max]");
        return false;
    }
    return true;
}

bool isValidServerName::checkSegmentLength(const std::string& value) const {
    std::stringstream ss(value);
    std::string segment;
    while (std::getline(ss, segment, '.')) {
        if (segment.length() > 63) {
            throw std::invalid_argument("ServerName contains a domain that is too long [63 characters max]: " + segment);
            return false;
        }
    }
    return true;
}

bool isValidServerName::checkNumberOfDots(const std::string& value) const {
    if (std::count(value.begin(), value.end(), '.') < 1) {
        throw std::invalid_argument("ServerName does not contain a domain");
        return false;
    }
    return true;
}

bool isValidServerName::checkConsecutiveDots(const std::string& value) const {
    if (value.find("..") != std::string::npos) {
        throw std::invalid_argument("ServerName contains consecutive dots");
        return false;
    }
    return true;
}

bool isValidServerName::validate(const std::string& value, const std::map<std::string, std::string>& fieldProperties) const {
    (void)fieldProperties; // Currently unused

    return checkNonEmptiness(value) &&
           checkValidCharacters(value) &&
           checkWildcardUsage(value) &&
           checkDomainLength(value) &&
           checkSegmentLength(value) &&
           checkNumberOfDots(value) &&
           checkConsecutiveDots(value);
}