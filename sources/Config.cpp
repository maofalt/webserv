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

Config::Config() : _tokens(" \t\n;{}"), _whiteSp(" \t"){
}

Config::~Config() {
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

bool	Config::basicCheck() { // ! need to refacto this monstruosity !
	int	bracketOpen = 0;
	int	countLines = 1;
	int	err = 0;

	for (std::vector<std::string>::iterator it = _splitContent.begin(); \
	it != _splitContent.end(); it++) {
		if (*it == "\n") {
			countLines++;
			if (it != _splitContent.begin() && *it == "\n" && *(it - 1) != ";"  && *(it - 1) != "\n" \
			&& *(it - 1) != "{"  && *(it - 1) != "}") {
				std::cerr << _confFileName + ": error: expected ';' line " << countLines - 1 << std::endl;
				err++;
			}
		}
		else if (*it == "{")
			bracketOpen++;
		else if (*it == "}")
			bracketOpen--;
		else if ((it + 1) != _splitContent.end() && *it == ";" && *(it + 1) != "\n") {
			std::cerr << _confFileName + ": error: expected '\\n' after ';' line " << countLines << std::endl;
			err++;
		}
		if (bracketOpen < 0) {
			return std::cerr << _confFileName + ": error: Extra closing bracket line " << std::endl, err;
		}
	}
	if (bracketOpen > 0) {
		return std::cerr << _confFileName + ": error: Missing closing bracket" << std::endl, err + 1;
	}
	return (err != 0 ? err : 0);
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

bool	Config::setupConf(std::ifstream & file, std::string fileName) {
	_confFileName = fileName;
	readConf(file);
	splitConf();
	if (basicCheck())
		return 1;

	// create config;

	return 0;
}

std::ostream& operator<<(std::ostream& os, const Config & conf) {
	for (size_t i=0; i<conf.getRawContent().size(); i++) {
		os << conf.getRawContent()[i];
	}
	std::cout << "==============================================================" << std::endl;
	for (size_t i=0; i<conf.getSplitContent().size(); i++) {
		os << "[" << conf.getSplitContent()[i] << "]";
		// os << std::endl;
	}
	return os;
}