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

std::vector< struct server >	Config::getServList() const {
	return this->_servList;
}

std::map< std::string, std::vector< std::string > >	Config::getConfData() const {
	return this->_confData;
}

int	Config::basicCheck() { // !!! need to refacto this monstruosity !!!
	int	bracketOpen = 0;
	int	countLines = 1;
	int	err = 0;

	for (std::vector<std::string>::iterator it = _splitContent.begin(); \
	it != _splitContent.end(); it++) {
		if (*it == "\n") {
			countLines++;
			if (it != _splitContent.begin() && *it == "\n" && *(it - 1) != ";"  && *(it - 1) != "\n" \
			&& *(it - 1) != "{"  && *(it - 1) != "}") {
				std::cerr << _confFileName + ":" << countLines - 1 << ": error: expected ';' before '\\n'." << std::endl;
				err++;
			}
		}
		else if (*it == "{") {
			if (it + 1 != _splitContent.end() && *(it + 1) != "\n") {
				std::cerr << _confFileName + ":" << countLines << ": error: expected '\\n' after '{'." << std::endl;
				err++;
			}
			bracketOpen++;
		}
		else if (*it == "}")
			bracketOpen--;
		else if ((it + 1) != _splitContent.end() && *it == ";" && *(it + 1) != "\n") {
			std::cerr << _confFileName + ":" << countLines << ": error: expected '\\n' after ';'." << std::endl;
			err++;
		}
		if (bracketOpen < 0) {
			return std::cerr << _confFileName + ": error: extra closing bracket." << std::endl, err;
		}
	}
	if (bracketOpen > 0) {
		return std::cerr << _confFileName + ": error: missing closing bracket." << std::endl, err + 1;
	}
	return err;
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

int	Config::parseLocConf(std::vector<std::string>::iterator & it, int & line, struct server & newServ) {
	int	err = 0;

	if (++it == _splitContent.end() || *it == ";" || *it == "\n" || *it == "{" || *it == "}") {
		line += (*it == "\n");
		std::cerr << _confFileName + ":" << line << ": error: missing path for location block." << std::endl;
		err++;
	}

	struct location	newLoc;

	while (it != _splitContent.end() && *it != ";" && *it != "\n" && *it != "{" && *it != "}")
		newLoc._paths.push_back(*(it++));
	if (it == _splitContent.end() || *it != "{") {
		line += (*it == "\n");
		return std::cerr << _confFileName + ":" << line << ": error: missing '{' for location block." << std::endl, 1;
		// err++;
	}

	while (++it != _splitContent.end() && *it != "}") {
		if (*it == "\n")
			line++;
		else {
			std::string	key = *it;
			while (++it != _splitContent.end() && *it != ";" && *it != "\n" && *it != "{" && *it != "}")
				newLoc._locConfig[key].push_back(*it);
		}
	}
	newServ._locations.push_back(newLoc);
	return err;
}

int	Config::parseServConf(std::vector<std::string>::iterator & it, int & line) {
	int	err = 0;

	if (*(++it) != "{") {
		line += (*it == "\n");
		return std::cerr << _confFileName + ":" << line << ": error: missing '{' for server block." << std::endl, 1;
		// err++;
	}
	
	struct server	newServ;

	while (++it != _splitContent.end() && *it != "}") {
		if (*it == "\n")
			line++;
		else if (*it == "location") {
			err += parseLocConf(it, line, newServ);
		}
		else {
			std::string	key = *it;
			while (++it != _splitContent.end() && *it != ";" && *it != "\n" && *it != "{" && *it != "}")
				newServ._servConfig[key].push_back(*it);
		}
	}
	_servList.push_back(newServ);
	return err;
}

int	Config::setupConf(std::ifstream & file, std::string fileName) {
	struct stat	fileStat;
	int	err = 0;
	_confFileName = fileName;

	if (stat(_confFileName.c_str(), &fileStat) != 0)
		return std::cerr << _confFileName + ": error: could not check file status." << std::endl, 1;
	if (fileStat.st_size > 2000 || fileStat.st_size == 0)
		return std::cerr << _confFileName + ": error: file is too big or empty." << std::endl, 1;

	readConf(file);
	splitConf();
	if (basicCheck())
		return 1;

	int	line = 1;
	for (std::vector<std::string>::iterator it = _splitContent.begin(); \
	it != _splitContent.end(); it++) {
		if (*it == "\n")
			line++;
		else if (*it == "server") {
			if (parseServConf(it, line))
				err++;
		}
		else {
			std::string	key = *it;
			while (++it != _splitContent.end() && *it != ";" && *it != "\n" && *it != "{" && *it != "}") {
				_confData[key].push_back(*it);
			}
			line += (*it == "\n");
		}
	}

	return err;
}

void	printLocStruct(std::ostream& os, struct location & loc) {
	os << "		path(s) : ";
	for (size_t i=0; i<loc._paths.size(); i++) {
		os << loc._paths[i] + (i == loc._paths.size() - 1 ? "" : ", ");
	}
	os << std::endl;
	for (std::map< std::string, std::vector< std::string > >::iterator \
	it = loc._locConfig.begin(); it != loc._locConfig.end(); it++) {
		os << "		" + it->first << " : ";
		for (size_t i=0; i<it->second.size(); i++) {
			os << it->second[i] << (i == it->second.size() - 1 ? "" : ", ");
		}
		os << std::endl;
	}
}

// !! CONFIG SETUP LEAKS WHEN DEFAULT FILES HAS ERROR TOO !! + WHEN NO '\n' AT END OF FILE : LAST LINE DUPLICATES !!

/*=========================================== DISPLAY FUNCTS ================================================*/

void	printServStruct(std::ostream& os, struct server & serv) {
	for (std::map< std::string, std::vector< std::string > >::iterator \
	it = serv._servConfig.begin(); it != serv._servConfig.end(); it++) {
		os << "	" + it->first + " : ";
		for (size_t i=0; i<it->second.size(); i++) {
			os << it->second[i] << (i == it->second.size() - 1 ? "" : ", ");
		}
		os << std::endl;
	}
	os << std::endl << "	Location(s) :" << std::endl;
	for (size_t i=0; i<serv._locations.size(); i++) {
		os << "		location " << i + 1 << " :" << std::endl;
		printLocStruct(os, serv._locations[i]);
		os << std::endl;
	}
}

std::ostream& operator<<(std::ostream& os, const Config & conf) {
	for (size_t i=0; i<conf.getSplitContent().size(); i++) {
		os << "[" << conf.getSplitContent()[i] << "]";
	}

	os << std::endl;
	os << "==========================================================================" << std::endl;
	os << std::endl;

	std::map< std::string, std::vector< std::string > >	config = conf.getConfData();
	std::map< std::string, std::vector< std::string > >::iterator \
	it = config.begin();
	os << "Config Data : " << std::endl << std::endl;
	while (it != config.end()) {
		os << it->first << " : ";
		for (size_t i=0; i<it->second.size(); i++) {
			os << it->second[i] << (i == it->second.size() - 1 ? "" : ", ");
		}
		it++;
		os << std::endl;
	}

	os << std::endl;
	os << "==========================================================================" << std::endl;
	os << std::endl;

	std::vector< struct server >	servers = conf.getServList();
	for (size_t i=0; i<servers.size(); i++) {
		os << "Server " << i + 1 << " :" << std::endl;
		printServStruct(os, servers[i]);
		os << std::endl;
	}

	os << std::endl;
	os << "==========================================================================" << std::endl;
	os << std::endl;

	return os;
}
