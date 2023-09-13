/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConf.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: znogueir <znogueir@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/26 16:47:13 by znogueir          #+#    #+#             */
/*   Updated: 2023/09/12 19:43:28 by znogueir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"
#include "ServerConf.hpp"

std::map< std::string, std::vector< std::string > >	ServerConfig::getConf() {
    return this->_servConfig;
}

ServerConfig	*Config::findServer(std::string server_name, uint16_t port)
{
	ServerConfig	*defaultServer;

	defaultServer = 0;
	for (std::vector< ServerConfig >::iterator it = _servList.begin(); it != _servList.end(); ++it)
	{
		if (!it->isListeningTo(port))
			continue ;
		if (!defaultServer)
			defaultServer = &(*it);
		if (it->isNamed(server_name))
			return (&(*it));
	}
	return (defaultServer);
}

bool ServerConfig::isListeningTo(uint16_t port) const
{
    char portStr[6];
    std::sprintf(portStr, "%u", port);

    if (_servConfig.count("listen"))
    {
        const std::vector<std::string>& ports = _servConfig.at("listen");
        // Start the loop from index 1 because index 0 contains the "listen" key
        for (std::vector<std::string>::const_iterator it = ports.begin() + 1; it != ports.end(); ++it)
        {
            if (*it == portStr)
            {
                return true;
            }
        }
    }
    return false;
}

bool ServerConfig::isNamed(const std::string &name) const
{
    if (_servConfig.count("server_name"))
    {
        const std::vector<std::string>& names = _servConfig.at("server_name");
        // Start the loop from index 1 because index 0 contains the "server_name" key
        for (std::vector<std::string>::const_iterator it = names.begin() + 1; it != names.end(); ++it)
        {
            if (*it == name)
            {
                return true;
            }
        }
    }
    return false;
}

//Fetch and send back a list of unique ports to listen
std::set<std::string>	ServerConfig::getPorts() {
	std::set<std::string> ports;

	//finds listen if present
	std::map<std::string, std::vector<std::string> >::iterator it = _servConfig.find("listen");
	if (it == _servConfig.end()) {return ports;}

	//adds all ports to the set such as we add only unique ports
	//we ignore first string as it is "listen"
	for (std::vector<std::string>::iterator it2 = it->second.begin() + 1;
		it2 != it->second.end(); ++it2) {
		ports.insert(*it2);
		log_message(Logger::DEBUG, "Port added: %s", std::string(*it2).c_str());
	}
	return ports;
}


