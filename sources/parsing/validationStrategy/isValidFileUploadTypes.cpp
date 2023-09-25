/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   isValidFileUploadTypes.cpp                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/02 21:19:07 by motero            #+#    #+#             */
/*   Updated: 2023/09/15 16:45:08 by motero           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "isValidFileUploadTypes.hpp"


bool isValidFileUploadTypes::validate(const std::vector<std::string>& values, const std::map<std::string, std::string>& fieldProperties) const {
    for (std::vector<std::string>::const_iterator it = values.begin(); it != values.end(); ++it) {
        if (!validate(*it, fieldProperties)) {
            return false; }
    }
    return true;
}

std::set<std::string> isValidFileUploadTypes::getValidTypes() const {
    std::set<std::string> validTypes;
    validTypes.insert("jpg");
    validTypes.insert("jpeg");
    validTypes.insert("png");
    validTypes.insert("gif");
    return validTypes;
}

std::vector<std::string> isValidFileUploadTypes::splitValue(const std::string& value) const {
    std::vector<std::string> givenTypes;
    std::stringstream ss(value);
    std::string item;
    while (std::getline(ss, item, ',')) {
        givenTypes.push_back(item);
    }
    return givenTypes;
}

bool isValidFileUploadTypes::checkAgainstValidTypes(const std::vector<std::string>& givenTypes, const std::set<std::string>& validTypes) const {
    for (std::vector<std::string>::const_iterator type = givenTypes.begin(); type != givenTypes.end(); ++type) {
        if (validTypes.find(*type) == validTypes.end()) {
            throw std::invalid_argument("Invalid file type: " + *type);
        }
    }
    return true;
}

bool isValidFileUploadTypes::validate(const std::string& value, const std::map<std::string, std::string>& fieldProperties) const {
    (void)fieldProperties; // Unused
    std::set<std::string> validTypes = getValidTypes();
    std::vector<std::string> givenTypes = splitValue(value);

    return checkAgainstValidTypes(givenTypes, validTypes);
}