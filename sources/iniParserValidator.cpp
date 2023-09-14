/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   iniParserValidator.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/30 16:11:41 by motero            #+#    #+#             */
/*   Updated: 2023/09/14 14:43:07 by motero           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "IniParser.hpp"

void    IniParser::initializeValidTypes() {
    _validTypes["integer"] =    &IniParser::isValidInteger;
    _validTypes["string"] =     &IniParser::isValidString;
    _validTypes["bool"] =       &IniParser::isValidBoolean;
    _validTypes["ipv4"] =       &IniParser::isValidIpv4;
    _validTypes["port"] =       &IniParser::isValidPort;
}

bool    IniParser::isDefaultValid(const std::string& value) {
    if (_validTypes.find(_currentType) != _validTypes.end()) {
        return _validTypes[_currentType](value);
    }

    return true;
}

bool    IniParser::isMaxValid(const std::string& value) {
    
    if (_validTypes.find(_currentType) != _validTypes.end()) {
        return _validTypes[_currentType](value);
    }

    return true; 
}

bool    IniParser::isMinValid(const std::string& value) {
    if (_validTypes.find(_currentType) != _validTypes.end()) {
        return _validTypes[_currentType](value);
    }
    return true; 
}

bool IniParser::isValidInteger(const std::string& value) {
    if (!isInteger(value)) {
        return false;
    }
    std::stringstream ss(value);
    int i;
    ss >> i;
    if (ss.fail()) {
        ss.clear(); 
        return false;
    }
    return true;
}


bool IniParser::isValidString(const std::string& value) {
    if (value.empty()) {
        return false;
    }
    return true;
}

bool IniParser::isValidBoolean(const std::string& value) {
    return (value == "true"  || value == "false");
}

bool IniParser::isValidIpv4(const std::string& value) {
    std::istringstream ss(value);
    int a, b, c, d;
    char ch1, ch2, ch3;

    ss >> a >> ch1 >> b >> ch2 >> c >> ch3 >> d;

    if (!(ch1 == '.' && ch2 == '.' && ch3 == '.')) {
        return false;
    }

    if (a < 0 || a > 255 || b < 0 || b > 255 || c < 0 || c > 255 || d < 0 || d > 255) {
        return false;
    }

    return ss.eof();  // Make sure we consumed the whole string
}


bool IniParser::isValidPort(const std::string& value) {
    std::istringstream ss(value);
    int port;
    ss >> port;
    if (port < 0 || port > 65535) {
        return false;
    }
    return ss.eof();  // Make sure we consumed the whole string
}