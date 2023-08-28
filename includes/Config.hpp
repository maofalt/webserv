/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: znogueir <znogueir@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/20 14:42:02 by znogueir          #+#    #+#             */
/*   Updated: 2023/08/28 17:39:43 by znogueir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
# define CONFIG_HPP

#include <map>
#include <vector>
#include <cstdlib>
#include <fstream>
#include <stdint.h>
#include <iostream>
#include <sys/stat.h>
#include "Logger.hpp"
#include "Colors.hpp"
#include "ServerConf.hpp"

# define DISPLAY_CONF 1

class Config {
	private:
		int											_nbrLines;
		int											_nbrErr;
		std::string									_tokens;
		std::string									_whiteSp;
		std::string									_confFileName;
		// size_t										_fileSize;
		std::vector< std::string >					_rawContent;
		std::vector< std::string >					_splitContent;
		std::vector< ServerConfig >						_servList;
		std::map< std::string, std::vector< std::string > >	_confData;

	public:
		// missing coplien !!!!;
		Config();
		~Config();

		// error printing;
		int		printNbErr();
		void	printErr(std::string errMsg, int line);

		// reading and splitting file
		void	readConf(std::ifstream & file);
		void	splitConf();

		// basic syntax check
		void	checkRetToL(std::vector<std::string>::iterator & it,
				int & countLines);
		void	checkOpenBrack(std::vector<std::string>::iterator & it,
				int & countLines, int & bracketOpen);
		void	checkCloseBrack(std::vector<std::string>::iterator & it,
				int & countLines, int & bracketOpen);
		void	checkSemiCol(std::vector<std::string>::iterator & it,
				int & countLines);
		int		basicCheck();

		// filling the data struct for server and location blocks
		void	pushToStructMap(std::vector<std::string>::iterator & it,
				std::map< std::string, std::vector<std::string> > & sMap, int & line);
		int		parseLocConf(std::vector<std::string>::iterator & it,
				int & line, ServerConfig & newServ);
		int		parseLocConf2(std::vector<std::string>::iterator & it,
				int & line, ServerConfig & newServ, struct location &newLoc);
		int		parseServConf(std::vector<std::string>::iterator & it, int & line);
		int		parseServConf2(std::vector<std::string>::iterator & it,
				int & line, ServerConfig & newServ);

		// main functions
		int		fillStruct(int line, std::vector<std::string>::iterator & it);
		int		setupConf(std::ifstream & file, std::string fileName);

		// getters
		std::vector<std::string>	getRawContent() const;
		std::vector<std::string>	getSplitContent() const;
		std::vector< ServerConfig >	getServList() const;
		std::map< std::string, std::vector< std::string > >	getConfData() const;

		// specific server getter
		const ServerConfig	*findServer(std::string server_name, uint16_t port) const;
};

std::ostream& operator<<(std::ostream& os, const Config & conf);

#endif
