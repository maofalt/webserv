/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   isValidDirectoryPath.cpp                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/02 21:19:07 by motero            #+#    #+#             */
/*   Updated: 2023/09/15 15:26:28 by motero           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ValidationStrategy.hpp"


bool isValidDirectoryPath::validate(const std::vector<std::string>& values, const std::map<std::string, std::string>& fieldProperties) const {
    for (std::vector<std::string>::const_iterator it = values.begin(); it != values.end(); ++it) {
        if (!validate(*it, fieldProperties)) {
            return false; }
    }
    return true;
}

bool isValidDirectoryPath::validate(const std::string& value, const std::map<std::string, std::string>& fieldProperties) const {
    struct stat s;
    (void)fieldProperties;
    if(stat(value.c_str(), &s) == 0) {
        if(s.st_mode & S_IFDIR) {
            // It's a directory
            return true;
        } else if(s.st_mode & S_IFREG) {
            throw std::invalid_argument("Path is a file, not a directory");
            return false;
        } else {
            throw std::invalid_argument("Path is not a file or directory (i think....)");
            return false;
        }
    } else {
        throw std::invalid_argument("Directory does not exist or don't have permissions");
        // Error could be due to many reasons like file/directory doesn't exist or isn't accessible.
        return false;
    }
}
