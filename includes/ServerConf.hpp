/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConf.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/26 16:47:00 by znogueir          #+#    #+#             */
/*   Updated: 2023/08/29 22:52:51 by motero           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef	SERVERCONF_HPP
# define SERVERCONF_HPP

#include <map>
#include <vector>
#include <iostream>

/*
1-valdiate Global config
*/

//location cn be recursive creating routes, thus we need a struct for
struct GlobalConfig {
    std::string defaultErrorPages;      // Path or directory for default error pages.
	int			clientBodyLimit;		// Maximum size of the body of a request.
	int			clientHeaderLimit;		// Maximum size of the header of a request.
	int			timeout;					// Maximum time to wait for a request.
	int			maxConnections;				// Maximum number of connections.
	int			maxRequests;				// Maximum number of requests.
	int			maxRequestsPerIP;			// Maximum number of requests per IP.
};

//
struct location {
	std::vector< std::string >								_paths;
	std::map< std::string, std::vector< std::string > >	_locConfig;
	//is this a good diea 
	std::string											_path;
    std::vector<std::string>							_httpMethods;
    std::string											_redirectionTarget;
    std::string											_directoryRoot;
    bool												_directoryListing;
    std::string											_defaultFile;
    std::string											_cgiExtensions;
    std::string											_uploadDir;
    std::vector<std::string>							_uploadSettings;
};

class ServerConfig {
	public:
		std::map< std::string, std::vector< std::string > >	_servConfig;
		std::vector< struct location >						_locations;

		bool	isListeningTo(uint16_t port) const;
		bool	isNamed(const std::string &name) const;
	//i would like these structures but i do not kmow if romain uses them 
	uint16_t											_port;
	std::string											_host;
	std::vector<std::string>							_serverNames;
	int													_clientBodyLimit;
	std::string											_errorPages;
};

#endif
