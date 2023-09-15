/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   isValidMethod.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/02 21:19:07 by motero            #+#    #+#             */
/*   Updated: 2023/09/15 15:25:56 by motero           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ValidationStrategy.hpp"


bool isValidMethod::validate(const std::vector<std::string>& values, const std::map<std::string, std::string>& fieldProperties) const {
    for (std::vector<std::string>::const_iterator it = values.begin(); it != values.end(); ++it) {
        if (!validate(*it, fieldProperties)) {
            return false; }
    }
    return true;
}

bool isValidMethod::validate(const std::string& value, const std::map<std::string, std::string>& fieldProperties) const {
    std::string methods[] = {"GET", "POST", "DELETE"};
    (void)fieldProperties;
    for(int i = 0; i < 3; ++i) {
        if(value == methods[i]) {
            return true;
        }
    }
    throw std::invalid_argument("Method " + value + " is not valid");
    return false;
}
