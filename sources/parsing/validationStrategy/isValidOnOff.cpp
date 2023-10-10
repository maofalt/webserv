/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   isValidOnOff.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/02 21:19:07 by motero            #+#    #+#             */
/*   Updated: 2023/09/15 16:26:11 by motero           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "isValidOnOff.hpp"


bool isValidOnOff::validate(const std::vector<std::string>& values, const std::map<std::string, std::string>& fieldProperties) const{
    for (std::vector<std::string>::const_iterator it = values.begin(); it != values.end(); ++it) {
        if (!validate(*it, fieldProperties)) {
            return false; }
    }
    return true;
}

bool isValidOnOff::validate(const std::string& value, const std::map<std::string, std::string>& fieldProperties) const {
    (void)fieldProperties;
    
    std::vector<std::string> validValues;
    validValues.push_back("on");
    validValues.push_back("off");
    if (std::find(validValues.begin(), validValues.end(), value) == validValues.end()) {
        throw std::invalid_argument("Invalid value for OnOff: " + value);
        return false;
    }

    return true;
}