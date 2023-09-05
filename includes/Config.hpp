/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/20 14:42:02 by znogueir          #+#    #+#             */
/*   Updated: 2023/09/05 17:55:56 by motero           ###   ########.fr       */
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
#include "Logger.hpp"
#include "IniParser.hpp"

# define DISPLAY_CONF 1

class Config {
	private:
		int											_nbrLines;
		int											_nbrErr;
		std::string									_tokens;
		std::string									_whiteSp;
		std::string									_confFileName;
		std::vector< std::string >					_rawContent;
		std::vector< std::string >					_splitContent;
		std::vector< ServerConfig >						_servList;
		std::map< std::string, std::vector< std::string > >	_confData;
		IniParser*									_validationFile;

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

		//validate the data struct for server and location blocks
		bool	validateConfig();
		void	validateValue(const std::string& fullContext, std::vector<std::string>& values, const std::map<std::string, std::string>& fieldProperties);
        bool	validateGlobalConfig();
        bool	validateVirtualServerConfig();
        bool	validateLocationConfig(std::vector<location>& locations);
		void	handleDuplicateValues(std::vector<std::string>& values, const std::map<std::string, std::string>& fieldProperties);
		std::map<std::string, std::string>	getFieldProperties(const std::string& context);

		//Helper methdos for validating data and structure more the code 
		bool	validateConfigData(std::map<std::string, std::vector<std::string> >& confData, const std::string& contextType);
		bool	validateMandatoryKeys(const std::map<std::string, std::vector<std::string> >& confData, const std::string& contextType);
		bool 	validateMandatoryKeys(const std::map<std::string, std::string>& confData, const std::string& contextType);

		// main functions
		int		fillStruct(int line, std::vector<std::string>::iterator & it);
		int		setupConf(std::ifstream & file, std::string fileName);

		// getters
		std::vector<std::string>	getRawContent() const;
		std::vector<std::string>	getSplitContent() const;
		std::vector< ServerConfig >	getServList() const;
		std::map< std::string, std::vector< std::string > >	getConfData() const;

		//dependency injection through setter
		void	setValidationFile(IniParser* validationFile);

		// specific server getter
		const ServerConfig	*findServer(std::string server_name, uint16_t port) const;
};

std::ostream& operator<<(std::ostream& os, const Config & conf);

#endif
