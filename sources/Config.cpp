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

Config::Config(/* args */) : _tokens(" \t\n;{}"), _whiteSp(" \t"){
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
		_rawContent.push_back(line + "\n");
	}
	file.close();
}

std::vector<std::string>	Config::getRawContent() const {
	return this->_rawContent;
}

std::vector<std::string>	Config::getSplitContent() const {
	return this->_splitContent;
}


void	Config::rmWhiteSpaces()
{
	// size_t	countSemiCol;

	// for (size_t i=0; i<_splitContent.size(); i++) {
	// 	countSemiCol = 0;
	// 	while (1) {
	// 		start = _splitContent[i].find_first_of(";");
	// 		if (start == std::string::npos)
	// 			break;
	// 		_splitContent[i].erase(start, 1);
	// 		_splitContent.insert(_splitContent.begin() + i + 1, ";");
	// 		countSemiCol++;
	// 	}
	// 	i += countSemiCol;
	// }
}

void	Config::splitConf() {
	size_t	size = _rawContent.size();
	size_t	start;
	size_t	end;

	for (size_t i=0; i<size; i++) {
		while (!_rawContent[i].empty()) {
			start = _rawContent[i].find_first_not_of(_tokens);
			if (start != std::string::npos) {
				end = _rawContent[i].find_first_of(_tokens, start);
				if (end != std::string::npos) {
					_splitContent.push_back(_rawContent[i].substr(start, end - start));
					while (_tokens.find(_rawContent[i][end]) != std::string::npos) {
						if (_whiteSp.find(_rawContent[i][end]) == std::string::npos)
							_splitContent.push_back(_rawContent[i].substr(end, 1));
						end++;
					}
				}
			}
			else {
				int	j = 0;
				while (_tokens.find(_rawContent[i][j]) != std::string::npos) {
					if (_whiteSp.find(_rawContent[i][j]) == std::string::npos)
						_splitContent.push_back(_rawContent[i].substr(j, 1));
					j++;
				}
			}
			_rawContent[i].erase(0, end);
		}
	}
}

bool	Config::setupConf(std::ifstream & file) {
	readConf(file);
	splitConf();
	return 0;
}

std::ostream& operator<<(std::ostream& os, const Config & conf) {
	for (size_t i=0; i<conf.getRawContent().size(); i++) {
		os << conf.getRawContent()[i];
	}
	std::cout << "==============================================================" << std::endl;
	for (size_t i=0; i<conf.getSplitContent().size(); i++) {
		os << "[" << conf.getSplitContent()[i] << "]";// << std::endl;
	}
	return os;
}