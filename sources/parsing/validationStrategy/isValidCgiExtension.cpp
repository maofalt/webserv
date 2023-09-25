/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   isValidCgiExtension.cpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/02 21:19:07 by motero            #+#    #+#             */
/*   Updated: 2023/09/21 16:49:12 by rgarrigo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "isValidCgiExtension.hpp"


bool isValidCgiExtension::validate(const std::vector<std::string>& values, const std::map<std::string, std::string>& fieldProperties) const {
    for (std::vector<std::string>::const_iterator it = values.begin(); it != values.end(); ++it) {
        if (!validate(*it, fieldProperties)) {
            return false; }
    }
    return true;
}

bool isValidCgiExtension::validate(const std::string& value, const std::map<std::string, std::string>& fieldProperties) const {
    (void)fieldProperties; // Unused

    // 1. Check for known extensions
    if (value.find("/php-cgi") == std::string::npos && value.find("/python") == std::string::npos) {
        throw std::invalid_argument("Invalid CGI extension in path: " + value);
        return false;
    }

    // // 2. Ensure the path exists
    // struct stat buffer;
    // if (stat(value.c_str(), &buffer) != 0) {
    //     throw std::invalid_argument("Path does not exist: " + value);
    //     return false;
    // }

    // // 3. Ensure the path is a file
    // if (S_ISDIR(buffer.st_mode)) {
    //     throw std::invalid_argument("Path is a directory, not a file: " + value);
    //     return false;
    // }

    // 4. Check if the current user has execute rights on the file
    // if (access(value.c_str(), X_OK) != 0) {
    //     throw std::invalid_argument("No execute rights on the file: " + value);
    //     return false;
    // }

    return true;
}
