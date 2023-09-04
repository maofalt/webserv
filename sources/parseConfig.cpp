/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parseConfig.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/26 18:13:31 by znogueir          #+#    #+#             */
/*   Updated: 2023/09/04 19:43:54 by motero           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"

// !! CONFIG SETUP LEAKS WHEN DEFAULT FILES HAS ERROR TOO !! + WHEN NO '\n' AT END OF FILE : LAST LINE DUPLICATES !!

int	Config::basicCheck() {
	int	bracketOpen = 0;
	int	countLines = 1;

	for (std::vector<std::string>::iterator it = _splitContent.begin(); \
			it != _splitContent.end(); it++) {
		if (*it == "\n")
			checkRetToL(it, countLines);
		else if (*it == "{")
			checkOpenBrack(it, countLines, bracketOpen);
		else if (*it == "}")
			checkCloseBrack(it, countLines, bracketOpen);
		else if (*it == ";")
			checkSemiCol(it, countLines);
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

int	Config::fillStruct(int line, std::vector<std::string>::iterator & it) {
	while (it != _splitContent.end() && *it == "\n") {
		line++;
		it++;
	}
	while (it != _splitContent.end() && *it != ";" && *it != "{") {
		it++;
	}
	if (it != _splitContent.end() && *it == ";") {
		pushToStructMap(it, _confData, line);
	}
	else if (it != _splitContent.end() && *it == "{") {
		if (*(--it) != "server")
			return printErr("missing or unknown block instruction \
(expected 'server').", line), ++_nbrErr;
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
	if (fileStat.st_size > 5000 || fileStat.st_size == 0)
		return printErr("file is too big or empty.", -1), 1;

	_nbrErr = 0;
	readConf(file);
	splitConf();


	// for (size_t i=0; i<_splitContent.size(); i++) {
	// 	std::cerr << "[" << _splitContent[i] << "]";
	// }

	_nbrErr = basicCheck();
	if (_nbrErr)
		return printNbErr();

	std::vector<std::string>::iterator it = _splitContent.begin();
	fillStruct(1, it);
	return printNbErr(), _nbrErr;
}
