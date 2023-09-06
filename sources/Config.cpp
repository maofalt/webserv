/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/20 14:41:53 by znogueir          #+#    #+#             */
/*   Updated: 2023/09/06 14:26:58 by motero           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"

Config::Config() : _tokens(" \t\n;{}"), _whiteSp(" \t")
{
	std::memset(&_globalConfig, -1, sizeof(t_globalConfig));
}

Config::~Config() {
}

// MISSING COPLIEN !!

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

