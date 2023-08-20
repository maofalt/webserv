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
	// if error
	// file.close();
	return 0;
}

void	removeComments(std::string &line) {
	size_t	pos = line.find_first_of('#');

	if (pos != std::string::npos)
		line.erase(pos, line.size() - pos);
}

void	Config::readConf(std::ifstream & file) {
	std::string	line;

	while (file)
	{
		std::getline(file, line);
		removeComments(line);
		rawContent.push_back(line + "\n");
	}
	file.close();
}

void	Config::splitSemiCol()
{
	// size_t	countSemiCol;

	// for (size_t i=0; i<splitContent.size(); i++) {
	// 	countSemiCol = 0;
	// 	while (1) {
	// 		start = splitContent[i].find_first_of(";");
	// 		if (start == std::string::npos)
	// 			break;
	// 		splitContent[i].erase(start, 1);
	// 		splitContent.insert(splitContent.begin() + i + 1, ";");
	// 		countSemiCol++;
	// 	}
	// 	i += countSemiCol;
	// }
}

void	Config::splitConf() {
	size_t	size = rawContent.size();
	size_t	start;
	size_t	end;

	for (size_t i=0; i<size; i++) {
		while (!rawContent[i].empty()) {
			start = rawContent[i].find_first_not_of(" \t");
			if (start != std::string::npos) {
				end = rawContent[i].find_first_of(" \t", start);
				if (end != std::string::npos)
					splitContent.push_back(rawContent[i].substr(start, end - start));
				else
					splitContent.push_back(rawContent[i].substr(start, rawContent[i].size() - start));
			}
			rawContent[i].erase(0, end);
		}
	}
	splitSemiCol(); // need to put '\n' and ';' alone;
}

bool	Config::setupConf(std::ifstream & file) {
	readConf(file);
	splitConf();
	// if (checkSyntax())
	// 	return 1;
	// std::cout << rawContent << std::endl;
	return 0;
}

std::ostream& operator<<(std::ostream& os, const Config & conf) {
	for (size_t i=0; i<conf.getRawContent().size(); i++) {
		os << conf.getRawContent()[i];
	}
	std::cout << "==============================================================" << std::endl;
	for (size_t i=0; i<conf.getSplitContent().size(); i++) {
		os << conf.getSplitContent()[i] << std::endl;
	}
	return os;
}