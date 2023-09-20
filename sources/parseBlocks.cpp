/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parseBlocks.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/28 15:02:45 by znogueir          #+#    #+#             */
/*   Updated: 2023/09/20 18:06:53 by rgarrigo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"

void	Config::pushToStructMap(
	std::vector<std::string>::iterator & it,
	std::map< std::string, std::vector<std::string> > & sMap,
	int & line) {
	
	std::vector<std::string>::iterator it2 = it;
	while (it2 != _splitContent.begin() && *it2 != "\n") {
		it2--;
	}
	if (*(it2 + 1 - (it2 == _splitContent.begin())) == ";" || \
		*(it2 + 2 - (it2 == _splitContent.begin())) == ";") {
		_nbrErr++;
		printErr("cannot associate variable with value (missing or bad format).", line);
	}
	else {
		it2 += (*it2 == "\n");
		std::string	key = *it2;
		// it2++;
		while (it2 != _splitContent.end() && *it2 != ";") {
			sMap[key].push_back(*it2);
			it2++;
		}
		it = it2;
	}
}

int	Config::parseLocConf2(
	std::vector<std::string>::iterator & it, int & line,
	ServerConfig & newServ, struct location & newLoc) {

	while (++it != _splitContent.end() && *it == "\n") {
		line++;
	}
	while (it != _splitContent.end() && *it != ";" && *it != "{" && *it != "}") {
		it++;
	}
	if (it != _splitContent.end() && *it == ";") {
		pushToStructMap(it, newLoc._locConfig, line);
	}
	else if (it != _splitContent.end() && *it == "{") {
		return printErr("opening brackets in location block.", line), ++_nbrErr;
	}
	if (it == _splitContent.end() || (it != _splitContent.end() && *it == "}"))
		return _nbrErr;
	return parseLocConf2(it, line, newServ, newLoc), 0;
}

int	Config::parseLocConf(
	std::vector<std::string>::iterator & it,
	int & line, ServerConfig & newServ) {

	if (++it == _splitContent.end() || *it == "{") {
		line += (*it == "\n");
		printErr("missing path for location block.", line);
		_nbrErr++;
	}

	struct location	newLoc;
	while (it != _splitContent.end() && *it != "{")
		newLoc._paths.push_back(*(it++));

	int ret = parseLocConf2(it, line, newServ, newLoc);
	return newServ._locations.push_back(newLoc), ret;
}

int	Config::parseServConf2(
	std::vector<std::string>::iterator & it,
	int & line, ServerConfig & newServ) {

	while (it != _splitContent.end() && *it == "\n" && *it != "}") {
		line++;
		it++;
	}
	if (*it == "}")
		return  _nbrErr;
	while (it != _splitContent.end() && *it != ";" && *it != "{" && *it != "}") {
		it++;
	}
	if (it != _splitContent.end() && *it == ";") {
		pushToStructMap(it, newServ._servConfig, line);
	}
	else if (it != _splitContent.end() && *it == "{") {
		std::vector<std::string>::iterator it2 = it;
		while (--it2 != _splitContent.begin() && *it2 != "\n") {}
		if (*(++it2) != "location") {
			return printErr("missing or unknown block instruction \
in server block (expected 'location').", line), ++_nbrErr;
		}
		if (parseLocConf(it2, line, newServ))
			return _nbrErr;
		it = it2;
	}
	return parseServConf2(++it, line, newServ), 0;
}

int	Config::fill_credentials(ServerConfig & newServ, int & start) {
	if (newServ._servConfig.find("credentials") == newServ._servConfig.end())
		return std::cerr << "MISSING CREDENTIALS FILE !" << std::endl, 1;

	std::ifstream	file;
	std::string		line;
	std::string		login;
	std::string		role;
	std::string		password;
	std::size_t		pos;
	std::string		fileName;

	fileName = newServ._servConfig.at("credentials")[1];
	file.open(fileName.c_str(), std::fstream::in);
	if (!file) {
		std::cerr << BOLD << _confFileName + ": "<< RED << "error: " << RESET;
		std::cerr << "failed to open credentials file in server block line " << start << "." << std::endl;
		return 1;
	}
	while (file) {
		std::getline(file, line);
		if (line.empty())
			continue;
		pos = line.find_first_of(';');
		if (pos == std::string::npos)
			return 1;
		login = line.substr(0, pos);
		line.erase(0, pos + 1);
		pos = line.find_first_of(';');
		if (pos == std::string::npos)
			return 1;
		password = line.substr(0, pos);
		line.erase(0, pos + 1);
		role = line;
		newServ._credentials[login][role] = password;
	}
	file.close();
	return 0;
}

int	Config::parseServConf(std::vector<std::string>::iterator & it, int & line) {
	ServerConfig	newServ;
	int	start = line;

	it += 2;
	while (it != _splitContent.end() && *it == "\n") {
		line++;
		it++;
	}
	int ret = parseServConf2(it, line, newServ);
	ret += fill_credentials(newServ, start);
	return _servList.push_back(newServ), ret;
}
