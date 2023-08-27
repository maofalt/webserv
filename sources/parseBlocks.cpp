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
		while (++it2 != _splitContent.end() && *it2 != ";")
			sMap[key].push_back(*it2);
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

int	Config::parseServConf(std::vector<std::string>::iterator & it, int & line) {
	ServerConfig	newServ;

	it += 2;
	while (it != _splitContent.end() && *it == "\n") {
		line++;
		it++;
	}
	int ret = parseServConf2(it, line, newServ);
	return _servList.push_back(newServ), ret;
}
