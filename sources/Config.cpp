/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: znogueir <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/20 14:41:53 by znogueir          #+#    #+#             */
/*   Updated: 2023/08/20 14:41:55 by znogueir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"

Config::Config(/* args */) {
}

Config::~Config() {
}

bool	Config::checkNorm(std::ifstream & file) {
	(void)file;
	return 1;
}

void	Config::readConf(std::ifstream & file) {
	std::string line;
	
	while (file)
	{
		std::getline(file, line);
		rawContent += line + "\n";
	}
}

void	Config::setupConf(std::ifstream & file) {
	readConf(file);
	std::cout << rawContent << std::endl;
}