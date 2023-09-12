/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   printConfig.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: znogueir <znogueir@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/26 18:13:42 by znogueir          #+#    #+#             */
/*   Updated: 2023/09/12 20:14:38 by znogueir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"

/*========================================= printing error messages ==============================================*/

void	Config::printErr(std::string errMsg, int line) {
	if (line != -1) {
		std::cerr << BOLD << _confFileName + ":" << line << ": ";
		std::cerr << RED << "error: " << RESET << errMsg << std::endl;
	} else {
		std::cerr << BOLD << _confFileName + ": " << RED << "error: ";
		std::cerr << RESET << errMsg << std::endl;
	}
}

int	Config::printNbErr() {
	std::cerr << BOLD << _confFileName + ": " << (_nbrErr ? RED : GREEN);
	return std::cerr << _nbrErr << RESET << " error(s)." << std::endl, _nbrErr;
}

/*========================================= printing config structs ==============================================*/

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
	os << std::endl << "	Credentials :" << std::endl;
	for (std::map< std::string, std::map< std::string, std::string > >::iterator \
		it = serv._credentials.begin(); it != serv._credentials.end(); it++) {
		os << "		" + it->first + " : " << std::endl;
		for (std::map< std::string, std::string >::iterator \
			it2 = it->second.begin(); it2 != it->second.end(); it2++) {
			os << "			" + it2->first + "; " + it2->second << std::endl;
		}
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
