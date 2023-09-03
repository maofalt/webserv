/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConf.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/26 16:47:00 by znogueir          #+#    #+#             */
/*   Updated: 2023/09/03 17:19:31 by motero           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef	SERVERCONF_HPP
# define SERVERCONF_HPP

#include <map>
#include <vector>
#include <iostream>
#include "validationFactory.hpp"
#include "ValidationStrategy.hpp"

struct GlobalConfig {
    int clientBodyLimit;        // Maximum size of the body of a request.
    int clientHeaderLimit;      // Maximum size of the header of a request.
    int timeout;                // Maximum time to wait for a request.
    int maxConnections;         // Maximum number of connections.
    int maxRequests;            // Maximum number of requests.
    int maxRequestsPerIP;       // Maximum number of requests per IP.
};

//
struct location {
    std::vector<std::string> 						_paths;    
    std::map<std::string, std::vector<std::string> > _locConfig;

    std::vector<std::string> 						httpMethods;
    std::vector<std::string> 						redirectionTarget;
    std::vector<std::string> 						directoryRoot;
    std::vector<std::string> 						defaultFile;
    std::vector<std::string> 						cgiExtensions;
    std::vector<std::string> 						uploadDir;
    std::vector<std::string> 						uploadSettings;
    bool 											directoryListing;
};


class ServerConfig {
	public:
		std::map< std::string, std::vector< std::string > >	_servConfig;
		std::vector<struct location>								_locations;

    	int 												clientBodyLimit;                     // Limit on the client's request body.
		bool												isListeningTo(uint16_t port) const;
		bool												isNamed(const std::string &name) const;
		bool												directoryListing;
		std::vector<uint16_t> 								ports;
    	std::vector<std::string> 							host;                        // Server's hostname.
    	std::vector<std::string> 							serverNames;    // List of names the server should respond to.
    	std::map<std::string, std::string>					errorPages;   // Map of error codes to error page file paths.
    private:
        void                                                validateConfigValue(const std::string& fullContext, const std::vector<std::string>& values, const std::map<std::string, std::string>& fieldProperties);
};

#endif
