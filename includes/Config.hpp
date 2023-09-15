/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: znogueir <znogueir@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/20 14:42:02 by znogueir          #+#    #+#             */
/*   Updated: 2023/09/15 16:37:15 by znogueir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
# define CONFIG_HPP

#include <map>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <stdint.h>
#include <iostream>
#include <sys/stat.h>
#include "Logger.hpp"
#include "Colors.hpp"
#include "ServerConf.hpp"
#include "Logger.hpp"
#include "IniParser.hpp"

# define DISPLAY_CONF 1

class Config {
	private:
		int													_nbrLines;
		int													_nbrErr;
		std::string											_tokens;
		std::string											_whiteSp;
		std::string											_confFileName;
		std::vector< std::string >							_rawContent;
		std::vector< std::string >							_splitContent;
		std::vector< ServerConfig >							_servList;
		std::map< std::string, std::vector< std::string > >	_confData;
		t_globalConfig										_globalConfig;

	public:
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
		int		fill_credentials(ServerConfig & newServ, int & start);
		int		setupConf(std::ifstream & file, std::string fileName);

		// getters
		std::vector<std::string>	getRawContent() const;
		std::vector<std::string>	getSplitContent() const;
		std::vector< ServerConfig >	getServList() const;
		std::map< std::string, std::vector< std::string > >	getConfData() const;
		std::vector< ServerConfig >&	getServList();
		std::map< std::string, std::vector< std::string > >&	getConfData();


		// setter for global config
		void						setGlobalConfig(t_globalConfig& globalConfig);
		void						getGlobalConfig(t_globalConfig& globalConfig) const;
		const t_globalConfig&				getGlobalConfig() const;
		
		// specific server getter
		ServerConfig	*findServer(std::string server_name, uint16_t port);
};

std::ostream& operator<<(std::ostream& os, const Config & conf);

#endif
