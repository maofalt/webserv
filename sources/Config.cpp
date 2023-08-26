/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: znogueir <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/20 14:41:53 by znogueir          #+#    #+#             */
/*   Updated: 2023/08/26 14:26:14 by rgarrigo         ###   ########.fr       */
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

std::vector< ServerConfig >	Config::getServList() const {
	return this->_servList;
}

std::map< std::string, std::vector< std::string > >	Config::getConfData() const {
	return this->_confData;
}

void	Config::printErr(std::string errMsg, int line) {
	if (line != -1)
		std::cerr << _confFileName + ":" << line << errMsg << std::endl;
	else
		std::cerr << _confFileName + ":" << errMsg << std::endl;
}

int	Config::printNbErr(int err) {
	return std::cerr << _confFileName + ": " << err << " errors." << std::endl, err;
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
				printErr(": error: expected ';' before '\\n'.", countLines - 1);
				err++;
			}
		}
		else if (*it == "{") {
			if (it + 1 != _splitContent.end() && *(it + 1) != "\n") {
				printErr(": error: expected '\\n' after '{'.", countLines);
				err++;
			}
			bracketOpen++;
		}
		else if (*it == "}") {
			if (it + 1 != _splitContent.end() && *(it + 1) != "\n") {
				printErr(": error: expected '\\n' after '}'.", countLines);
				err++;
			}
			bracketOpen--;
		}
		else if ((it + 1) != _splitContent.end() && *it == ";" && *(it + 1) != "\n") {
			printErr(": error: expected '\\n' after ';'.", countLines);
			err++;
		}
		if (bracketOpen < 0) {
			return printErr(": error: extra closing bracket line ", countLines), printNbErr(++err);
		}
	}
	if (bracketOpen > 0) {
		return printErr(": error: missing closing bracket.", -1), printNbErr(++err);
	}
	_nbrLines = countLines;
	return printNbErr(err);
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

// int	Config::parseLocConf(std::vector<std::string>::iterator & it, int & line, ServerConfig & newServ) {
// 	int	err = 0;

// 	if (++it == _splitContent.end() || *it == ";" || *it == "\n" || *it == "{" || *it == "}") {
// 		line += (*it == "\n");
// 		printErr(": error: missing path for location block.", line);
// 		err++;
// 	}

// 	struct location	newLoc;

// 	while (it != _splitContent.end() && *it != ";" && *it != "\n" && *it != "{" && *it != "}")
// 		newLoc._paths.push_back(*(it++));
// 	if (it == _splitContent.end() || *it != "{") {
// 		line += (*it == "\n");
// 		printErr(": error: missing '{' for location block.", line);
// 		err++;
// 	}

// 	while (++it != _splitContent.end() && *it != "}") {
// 		if (*it == "\n")
// 			line++;
// 		else {
// 			std::string	key = *it;
// 			while (++it != _splitContent.end() && *it != ";" && *it != "\n" && *it != "{" && *it != "}")
// 				newLoc._locConfig[key].push_back(*it);
// 		}
// 	}
// 	newServ._locations.push_back(newLoc);
// 	return err;
// }

int	Config::parseLocConf2(std::vector<std::string>::iterator & it, int & line, ServerConfig & newServ, struct location & newLoc) {
	int	err = 0;

	while (it != _splitContent.end() && *it != "{")
		newLoc._paths.push_back(*(it++));
	while (++it != _splitContent.end() && *it == "\n") {
		line++;
	}
	while (it != _splitContent.end() && *it != ";" && *it != "{" && *it != "}") { it++; }
	std::vector<std::string>::iterator it2 = it;
	if (it != _splitContent.end() && *it == ";") {
		while (--it2 != _splitContent.begin() && *it2 != "\n") {}
		if (*it2 == ";" || *(it2 + 1) == ";") {
			err++;
			printErr(": error: locConf cannot associate variable with value (missing or bad format).", line);
		}
		else {
			it2 += (*it2 == "\n");
			std::string	key = *it2;
			while (++it2 != _splitContent.end() && *it2 != ";")
				newLoc._locConfig[key].push_back(*it2);
			it = it2;
		}
	}
	else if (it != _splitContent.end() && *it == "{") {
		printErr(": error: locConf cannot associate variable with value (missing or bad format).", line);
		return ++err;
	}
	if (it == _splitContent.end() || (it != _splitContent.end() && *it == "}"))
		return err;
	return err + parseServConf2(++it, line, newServ);
}

int	Config::parseLocConf(std::vector<std::string>::iterator & it, int & line, ServerConfig & newServ) {
	int	err = 0;

	if (++it == _splitContent.end() || *it == "{") {
		line += (*it == "\n");
		printErr(": error: missing path for location block.", line);
		err++;
	}

	struct location	newLoc;
	err += parseLocConf2(it, line, newServ, newLoc);
	return newServ._locations.push_back(newLoc), err;
}

int	Config::parseServConf2(std::vector<std::string>::iterator & it, int & line, ServerConfig & newServ) {
	int	err = 0;

	while (it != _splitContent.end() && *it == "\n" && *it != "}") {
		line++;
		it++;
	}
	if (*it == "}")
		return  err;
	while (it != _splitContent.end() && *it != ";" && *it != "{" && *it != "}") { it++; }
	std::vector<std::string>::iterator it2 = it;
	if (it != _splitContent.end() && *it == ";") {
		while (--it2 != _splitContent.begin() && *it2 != "\n") {}
		if (*it2 == ";" || *(it2 + 1) == ";") {
			err++;
			printErr(": error: servConf cannot associate variable with value (missing or bad format).", line);
		}
		else {
			it2 += (*it2 == "\n");
			std::string	key = *it2;
			while (++it2 != _splitContent.end() && *it2 != ";")
				newServ._servConfig[key].push_back(*it2);
			it = it2;
		}
	}
	else if (it != _splitContent.end() && *it == "{") {
		while (--it2 != _splitContent.begin() && *it2 != "\n") {}
		if (*(++it2) != "location") {
			printErr(": error: servConf missing or unknown block instruction in server block (expected 'location').", line);
		}
		if (parseLocConf(it2, line, newServ))
			return ++err;
		it = it2;
	}
	return err + parseServConf2(++it, line, newServ);
}

int	Config::parseServConf(std::vector<std::string>::iterator & it, int & line) {
	int	err = 0;

	ServerConfig	newServ;

	it += 2;
	while (it != _splitContent.end() && *it == "\n") {
		std::cout << "serv it : " + *it + ", " + *(it - 1) + ", " << line << std::endl;
		line++;
		it++;
	}
	err += parseServConf2(it, line, newServ);
	return _servList.push_back(newServ), err;
}

int	Config::fillStruct(int line, int err, std::vector<std::string>::iterator & it) {
	while (it != _splitContent.end() && *it == "\n") {
		line++;
		it++;
	}
	while (it != _splitContent.end() && *it != ";" && *it != "{") { it++; }
	if (it != _splitContent.end() && *it == ";") {
		std::vector<std::string>::iterator it2 = it;
		while (--it2 != _splitContent.begin() && *it2 != "\n") {}
		if (*it2 == ";" || *(it2 + 1) == ";") { //countWords < 2 - (it2 == _splitContent.begin())) {
			err++;
			std::cerr << _confFileName + ":" << line << ": error: filling cannot associate variable with value (missing or bad format)." << std::endl;
		}
		else {
			it2 += (*it2 == "\n");
			std::string	key = *it2;
			while (++it2 != _splitContent.end() && *it2 != ";") {
				_confData[key].push_back(*it2);
			}
		}
	}
	else if (it != _splitContent.end() && *it == "{") {
		if (*(--it) != "server")
			return std::cerr << _confFileName + ":" << line << ": error: filling missing or unknown block instruction (expected 'server')." << std::endl, 1;
		if (parseServConf(it, line))
			return err++;
	}
	if (it == _splitContent.end())
		return printErr(": ", -1), std::cerr << err << " errors." << std::endl, err;
	return err + fillStruct(line, err, ++it);
}

int	Config::setupConf(std::ifstream & file, std::string fileName) {
	struct stat	fileStat;
	_confFileName = fileName;

	if (stat(_confFileName.c_str(), &fileStat) != 0)
		return std::cerr << _confFileName + ": error: could not check file status." << std::endl, 1;
	if (fileStat.st_size > 2000 || fileStat.st_size == 0)
		return std::cerr << _confFileName + ": error: file is too big or empty." << std::endl, 1;

	readConf(file);
	splitConf();
	if (basicCheck())
		return 1;

	std::vector<std::string>::iterator it = _splitContent.begin();
	return printNbErr(fillStruct(1, 0, it));
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

void	printServStruct(std::ostream& os, ServerConfig & serv) {
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

	std::vector< ServerConfig >	servers = conf.getServList();
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
