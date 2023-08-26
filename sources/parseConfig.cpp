/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parseConfig.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: znogueir <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/26 18:13:31 by znogueir          #+#    #+#             */
/*   Updated: 2023/08/26 18:13:33 by znogueir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"

// !! CONFIG SETUP LEAKS WHEN DEFAULT FILES HAS ERROR TOO !! + WHEN NO '\n' AT END OF FILE : LAST LINE DUPLICATES !!

int	Config::basicCheck() { // !!! need to refacto this monstruosity !!!
	int	bracketOpen = 0;
	int	countLines = 1;

	for (std::vector<std::string>::iterator it = _splitContent.begin(); \
			it != _splitContent.end(); it++) {
		if (*it == "\n") {
			countLines++;
			if (it != _splitContent.begin() && *it == "\n" && *(it - 1) != ";"  && *(it - 1) != "\n" \
					&& *(it - 1) != "{"  && *(it - 1) != "}") {
				printErr("expected ';' before '\\n'.", countLines - 1);
				_nbrErr++;
			}
		}
		else if (*it == "{") {
			if (it + 1 != _splitContent.end() && *(it + 1) != "\n") {
				printErr("expected '\\n' after '{'.", countLines);
				_nbrErr++;
			}
			bracketOpen++;
		}
		else if (*it == "}") {
			if (it + 1 != _splitContent.end() && *(it + 1) != "\n") {
				printErr("expected '\\n' after '}'.", countLines);
				_nbrErr++;
			}
			else if (it != _splitContent.begin() && *(it - 1) != "\n") {
				printErr("expected '\\n' before '}'.", countLines);
				_nbrErr++;
			}
			bracketOpen--;
		}
		else if ((it + 1) != _splitContent.end() && *it == ";" && *(it + 1) != "\n") {
			printErr("expected '\\n' after ';'.", countLines);
			_nbrErr++;
		}
		if (bracketOpen < 0) {
			return printErr("extra closing bracket", countLines), ++_nbrErr;
		}
	}
	if (bracketOpen > 0) {
		return printErr("missing closing bracket.", -1), ++_nbrErr;
	}
	_nbrLines = countLines;
	return _nbrErr;
}

int	Config::parseLocConf2(std::vector<std::string>::iterator & it, int & line, ServerConfig & newServ, struct location & newLoc) {
	while (++it != _splitContent.end() && *it == "\n") {
		line++;
	}
	while (it != _splitContent.end() && *it != ";" && *it != "{" && *it != "}") {
		it++;
	}
	std::vector<std::string>::iterator it2 = it;
	if (it != _splitContent.end() && *it == ";") {
		while (it2 != _splitContent.begin() && *it2 != "\n") { it2--;}
		if (*(it2 + 1) == ";" || *(it2 + 2) == ";") {
			_nbrErr++;
			printErr("cannot associate variable with value (missing or bad format).", line);
		}
		else {
			it2 += (*it2 == "\n");
			std::string	key = *it2;
			while (++it2 != _splitContent.end() && *it2 != ";") {
				newLoc._locConfig[key].push_back(*it2);
			}
			it = it2;
		}
	}
	else if (it != _splitContent.end() && *it == "{") {
		return printErr("opening brackets in location block.", line), ++_nbrErr;
	}
	if (it == _splitContent.end() || (it != _splitContent.end() && *it == "}"))
		return _nbrErr;
	return parseLocConf2(it, line, newServ, newLoc), _nbrErr;
}

int	Config::parseLocConf(std::vector<std::string>::iterator & it, int & line, ServerConfig & newServ) {
	if (++it == _splitContent.end() || *it == "{") {
		line += (*it == "\n");
		printErr("missing path for location block.", line);
		_nbrErr++;
	}

	struct location	newLoc;
	while (it != _splitContent.end() && *it != "{")
		newLoc._paths.push_back(*(it++));

	parseLocConf2(it, line, newServ, newLoc);
	return newServ._locations.push_back(newLoc), _nbrErr;
}

int	Config::parseServConf2(std::vector<std::string>::iterator & it, int & line, ServerConfig & newServ) {
	while (it != _splitContent.end() && *it == "\n" && *it != "}") {
		line++;
		it++;
	}
	if (*it == "}")
		return  _nbrErr;
	while (it != _splitContent.end() && *it != ";" && *it != "{" && *it != "}") { it++; }
	std::vector<std::string>::iterator it2 = it;
	if (it != _splitContent.end() && *it == ";") {
		while (it2 != _splitContent.begin() && *it2 != "\n") { it2--; }
		if (*(it2 + 1) == ";" || *(it2 + 2) == ";") {
			_nbrErr++;
			printErr("cannot associate variable with value (missing or bad format).", line);
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
			return printErr("missing or unknown block instruction in server block (expected 'location').", line), ++_nbrErr;
		}
		if (parseLocConf(it2, line, newServ))
			return _nbrErr;
		it = it2;
	}
	return parseServConf2(++it, line, newServ), _nbrErr;
}

int	Config::parseServConf(std::vector<std::string>::iterator & it, int & line) {
	ServerConfig	newServ;

	it += 2;
	while (it != _splitContent.end() && *it == "\n") {
		line++;
		it++;
	}
	parseServConf2(it, line, newServ);
	return _servList.push_back(newServ), _nbrErr;
}

int	Config::fillStruct(int line, std::vector<std::string>::iterator & it) {
	while (it != _splitContent.end() && *it == "\n") {
		line++;
		it++;
	}
	while (it != _splitContent.end() && *it != ";" && *it != "{") { it++; }
	if (it != _splitContent.end() && *it == ";") {
		std::vector<std::string>::iterator it2 = it;
		while (it2 != _splitContent.begin() && *it2 != "\n") { it2--; }
		if (*(it2 + 1 - (it2 == _splitContent.begin())) == ";" || *(it2 + 2 - (it2 == _splitContent.begin())) == ";") {
			_nbrErr++;
			printErr("fill cannot associate variable with value (missing or bad format).", line);
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
			return printErr("missing or unknown block instruction (expected 'server').", line), ++_nbrErr;
		if (parseServConf(it, line))
			return _nbrErr;
	}
	if (it == _splitContent.end())
		return _nbrErr;
	return fillStruct(line, ++it);
}

int	Config::setupConf(std::ifstream & file, std::string fileName) {
	struct stat	fileStat;
	_confFileName = fileName;

	if (stat(_confFileName.c_str(), &fileStat) != 0)
		return printErr("could not check file status.", -1), 1;
	if (fileStat.st_size > 2000 || fileStat.st_size == 0)
		return printErr("file is too big or empty.", -1), 1;

	_nbrErr = 0;
	readConf(file);
	splitConf();
	_nbrErr = basicCheck();
	if (_nbrErr)
		return printNbErr();

	std::vector<std::string>::iterator it = _splitContent.begin();
	fillStruct(1, it);
	return printNbErr(), _nbrErr;
}
