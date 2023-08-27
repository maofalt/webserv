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
		// std::cout << line << std::endl;
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
	if (fileStat.st_size > 2000 || fileStat.st_size == 0)
		return printErr("file is too big or empty.", -1), 1;

	_nbrErr = 0;
	readConf(file);
	splitConf();


	_nbrErr = basicCheck();
	std::cout << "lines : " << _nbrLines << std::endl;
	if (_nbrErr)
		return printNbErr();

	std::vector<std::string>::iterator it = _splitContent.begin();
	// std::cout << "test" << std::endl;
	fillStruct(1, it);
	// std::cout << "test" << std::endl;
	return printNbErr(), _nbrErr;
}
