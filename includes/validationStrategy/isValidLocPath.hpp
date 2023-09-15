/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   isValidLocPath.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/02 21:19:07 by motero            #+#    #+#             */
/*   Updated: 2023/09/15 15:26:16 by motero           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ValidationStrategy.hpp"


bool isValidLocPath::validate(const std::vector<std::string>& values, const std::map<std::string, std::string>& fieldProperties) const {
    for (std::vector<std::string>::const_iterator it = values.begin(); it != values.end(); ++it) {
        if (!validate(*it, fieldProperties)) {
            return false; }
    }
    return true;
}

bool isValidLocPath::validate(const std::string& value, const std::map<std::string, std::string>& fieldProperties) const {
    (void)&fieldProperties;
    // Check if the string is empty.
    if (value.empty()) {
        return false;
    }
    //check for root
    if (value[0] != '/') {
        return false;
    }
    // Check for acceptable characters in the path.
    // Here we're only allowing alphanumeric characters, '/', '.', '-', '_'.
    for (std::string::const_iterator it = value.begin(); it != value.end(); ++it) {
        if (!(std::isalnum(*it) || *it == '/' || *it == '.' || *it == '-' || *it == '_')) {
            return false;
        }
    }
    return true;
}
