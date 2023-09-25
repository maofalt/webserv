/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   isValidRedirect.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/02 21:19:07 by motero            #+#    #+#             */
/*   Updated: 2023/09/15 16:51:59 by motero           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "isValidRedirect.hpp"


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

    // Validate the URL part
    return validateURL(values[1]);
}

bool isValidRedirect::validate(const std::string& value, const std::map<std::string, std::string>& fieldProperties) const {
    (void)fieldProperties;
    return validateURL(value);
}


bool isValidRedirect::checkEmpty(const std::string& value) const {
    if (value.empty()) {
        throw std::invalid_argument("URL is empty");
        return false;
    }
    return true;
}

bool isValidRedirect::checkValidCharacters(const std::string& value) const {
    for (std::string::const_iterator it = value.begin(); it != value.end(); ++it) {
        if (!std::isalnum(*it) && *it != '-' && *it != '.' && *it != '*' && *it != '/' && *it != ':' && *it != '_') {
            throw std::invalid_argument(std::string("URL contains invalid characters: ") + *it);
            return false;
        }
    }
    return true;
}

bool isValidRedirect::checkWildcardUsage(const std::string& value) const {
    if (value.find('*') != std::string::npos) {
        throw std::invalid_argument("URL contains a wildcard");
        return false;
    }
    return true;
}

bool isValidRedirect::checkDomainLength(const std::string& value) const {
    if (value.length() > 2048) {
        throw std::invalid_argument("URL is too long [2048 characters max]");
        return false;
    }
    return true;
}

bool isValidRedirect::checkNumberOfDots(const std::string& value) const {
    if (std::count(value.begin(), value.end(), '.') < 1) {
        throw std::invalid_argument("URL does not contain a domain");
        return false;
    }
    return true;
}

bool isValidRedirect::checkConsecutiveDots(const std::string& value) const {
    if (value.find("..") != std::string::npos) {
        throw std::invalid_argument("URL contains consecutive dots");
        return false;
    }
    return true;
}

bool isValidRedirect::validateURL(const std::string& value) const {
    return checkEmpty(value) &&
           checkValidCharacters(value) &&
           checkWildcardUsage(value) &&
           checkDomainLength(value) &&
           checkNumberOfDots(value) &&
           checkConsecutiveDots(value);
}

