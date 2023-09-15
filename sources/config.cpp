/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: znogueir <znogueir@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/20 14:41:53 by znogueir          #+#    #+#             */
/*   Updated: 2023/09/15 16:37:07 by znogueir         ###   ########.fr       */
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
