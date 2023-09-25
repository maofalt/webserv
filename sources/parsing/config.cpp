/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/20 14:41:53 by znogueir          #+#    #+#             */
/*   Updated: 2023/09/18 16:33:25 by motero           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"

Config::Config() : _tokens(" \t\n;{}"), _whiteSp(" \t")
{
	//std::memset(&_globalConfig, -1, sizeof(t_globalConfig));
}

Config::~Config() {
}

std::vector<std::string>	Config::getRawContent() const {
	return this->_rawContent;
}

std::vector<std::string>	Config::getSplitContent() const {
	return this->_splitContent;
}

std::vector< ServerConfig >	Config::getServList() const {
	return this->_servList;
}

std::map< std::string, std::vector< std::string > >	Config::getConfData() const {
	return this->_confData;
}

std::vector< ServerConfig >&	Config::getServList() {
	return this->_servList;
}

std::map< std::string, std::vector< std::string > >&	Config::getConfData() {
	return this->_confData;
}


//Setter for global config
void	Config::setGlobalConfig(t_globalConfig& globalConfig) {
	this->_globalConfig = globalConfig;
}

void 	Config::getGlobalConfig(t_globalConfig& globalConfig) const {
	globalConfig = this->_globalConfig;
}

const t_globalConfig&	 Config::getGlobalConfig() const {
	return _globalConfig;
}

//getters for all global config attributes
int 	Config::getClientBodyLimit() const {
	const t_globalConfig& globalConfig = this->getGlobalConfig();
	(void)globalConfig;
	return this->_globalConfig.clientBodyLimit;
}

int	Config::getClientHeaderLimit() const {
	const t_globalConfig& globalConfig = this->getGlobalConfig();
	(void)globalConfig;
	return this->_globalConfig.clientHeaderLimit;
}

int	Config::getTimeoutClient() const {
	const t_globalConfig& globalConfig = this->getGlobalConfig();
	(void)globalConfig;
	return this->_globalConfig.timeoutClient;
}

int	Config::getTimeoutCgi() const {
	const t_globalConfig& globalConfig = this->getGlobalConfig();
	(void)globalConfig;
	return this->_globalConfig.timeoutCgi;
}

int	Config::getMaxConnections() const {
	const t_globalConfig& globalConfig = this->getGlobalConfig();
	(void)globalConfig;
	return this->_globalConfig.maxConnections;
}

int	Config::getMaxRequests() const {
	const t_globalConfig& globalConfig = this->getGlobalConfig();
	(void)globalConfig;
	return this->_globalConfig.maxRequests;
}

int	Config::getMaxRequestsPerIP() const {
	const t_globalConfig& globalConfig = this->getGlobalConfig();
	(void)globalConfig;
	return this->_globalConfig.maxRequestsPerIP;
}