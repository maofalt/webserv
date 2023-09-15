/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   isValidFileUploadTypes.cpp                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/02 21:19:07 by motero            #+#    #+#             */
/*   Updated: 2023/09/15 16:26:21 by motero           ###   ########.fr       */
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
