/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConf.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/26 16:47:00 by znogueir          #+#    #+#             */
/*   Updated: 2023/09/20 18:00:03 by rgarrigo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef	SERVERCONF_HPP
# define SERVERCONF_HPP


#include <map>
#include <vector>
#include <iostream>
#include "ValidationFactory.hpp"
#include "ValidationStrategy.hpp"

typedef struct GlobalConfig {
    int clientBodyLimit;        // Maximum size of the body of a request.
    int clientHeaderLimit;      // Maximum size of the header of a request.
    int timeoutClient;                // Maximum time to wait for a request.
    int timeoutCgi;                // Maximum time to wait for a request.
    int maxConnections;         // Maximum number of connections.
    int maxRequests;            // Maximum number of requests.
    int maxRequestsPerIP;       // Maximum number of requests per IP.
} t_globalConfig;

typedef struct location {
    std::vector<std::string> 						_paths;
    std::map<std::string, std::vector<std::string> > _locConfig;
} t_location;

class ServerConfig {
	public:
		std::map< std::string, std::vector< std::string > >			_servConfig;
		std::vector< struct location >								_locations;
		std::set<std::string>										_allowedMethods;
		std::map< std::string, std::map<std::string, std::string> >	_credentials;
		std::map< std::string, std::map<std::string, std::string> >	_sessionCookie;

		std::map< std::string, std::vector< std::string > >	getConf();
    	int 												clientBodyLimit;                     // Limit on the client's request body.
		bool												isListeningTo(uint16_t port) const;
		bool												isNamed(const std::string &name) const;
        std::set<std::string>                               getPorts();
};

#endif
