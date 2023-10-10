/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   configValidatorSetters.cpp                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/24 21:55:01 by rgarrigo          #+#    #+#             */
/*   Updated: 2023/09/14 14:34:16 by motero           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigValidator.hpp"

void ConfigValidator::setClientBodyLimit(const std::string& value) {
    std::stringstream ss(value);
    ss >> _globalConfig.clientBodyLimit;
}

void ConfigValidator::setClientHeaderLimit(const std::string& value) {
    std::stringstream ss(value);
    ss >> _globalConfig.clientHeaderLimit;
}

void ConfigValidator::setTimeoutClient(const std::string& value) {
    std::stringstream ss(value);
    ss >> _globalConfig.timeoutClient;
}

void ConfigValidator::setTimeoutCgi(const std::string& value) {
    std::stringstream ss(value);
    ss >> _globalConfig.timeoutCgi;
}

void ConfigValidator::setMaxConnections(const std::string& value) {
    std::stringstream ss(value);
    ss >> _globalConfig.maxConnections;
}

void ConfigValidator::setMaxRequests(const std::string& value) {
    std::stringstream ss(value);
    ss >> _globalConfig.maxRequests;
}

void ConfigValidator::setMaxRequestsPerIP(const std::string& value) {
    std::stringstream ss(value);
    ss >> _globalConfig.maxRequestsPerIP;
    log_message(Logger::DEBUG, "Max requests per IP set to %d", _globalConfig.maxRequestsPerIP);
}