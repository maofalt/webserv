/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: znogueir <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/20 14:42:02 by znogueir          #+#    #+#             */
/*   Updated: 2023/08/26 14:19:47 by rgarrigo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
# define CONFIG_HPP

#include <stdint.h>
#include <cstdlib>
#include <map>
#include <vector>
#include <fstream>
#include <iostream>
#include <sys/stat.h>

# define DISPLAY_CONF 1

struct location {
	std::vector< std::string >								_paths;
	std::map< std::string, std::vector< std::string > >	_locConfig;
};

class ServerConfig {
	public:
		std::map< std::string, std::vector< std::string > >	_servConfig;
		std::vector< struct location >						_locations;

		bool	isListeningTo(uint16_t port) const;
		bool	isNamed(const std::string &name) const;
};

class Config {
	private:
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

		void						readConf(std::ifstream & file);
		void						splitConf();
		int						basicCheck();
		int						parseLocConf(std::vector<std::string>::iterator & it, int & line, ServerConfig & newServ);
		int						parseServConf(std::vector<std::string>::iterator & it, int & line);
		int						setupConf(std::ifstream & file, std::string fileName);
		std::vector<std::string>	getRawContent() const;
		std::vector<std::string>	getSplitContent() const;
		std::vector< ServerConfig >	getServList() const;
		std::map< std::string, std::vector< std::string > >	getConfData() const;
		const ServerConfig		*findServer(std::string server_name, uint16_t port) const;
};

std::ostream& operator<<(std::ostream& os, const Config & conf);

#endif
