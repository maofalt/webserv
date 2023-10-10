/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   isValidPath.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/02 21:19:07 by motero            #+#    #+#             */
/*   Updated: 2023/09/15 16:26:07 by motero           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "isValidPath.hpp"


bool isValidPath::validate(const std::vector<std::string>& values, const std::map<std::string, std::string>& fieldProperties) const {
    for (std::vector<std::string>::const_iterator it = values.begin(); it != values.end(); ++it) {
        if (!validate(*it, fieldProperties)) {
            return false; }
    }
    return true;
}

bool isValidPath::validate(const std::string& value, const std::map<std::string, std::string>& fieldProperties) const{
    struct stat s;
    (void)fieldProperties;
    if(stat(value.c_str(), &s) == 0) {
        if(s.st_mode & S_IFREG) {
            return true;
        } else {
            throw std::invalid_argument("Path is not a file");
            return false;
        }
    } else {
        throw std::invalid_argument("File does not exist or don't have permissions. You stink");
        return false;
    }   
}
