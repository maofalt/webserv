/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConf.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/26 16:47:13 by znogueir          #+#    #+#             */
/*   Updated: 2023/09/03 17:21:47 by motero           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"
#include "ServerConf.hpp"

const ServerConfig	*Config::findServer(std::string server_name, uint16_t port) const
{
	const ServerConfig	*defaultServer;

	defaultServer = 0;
	for (std::vector< ServerConfig >::const_iterator it = _servList.begin(); it != _servList.end(); ++it)
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

bool	ServerConfig::isListeningTo(uint16_t port) const
{
	if (_servConfig.count("listen"))
		return (port == atoi(_servConfig.at("listen")[0].c_str()));
	return (false);
}

bool	ServerConfig::isNamed(const std::string &name) const
{
	if (_servConfig.count("server_name"))
		for (std::vector< std::string >::const_iterator it = _servConfig.at("server_name").begin(); it != _servConfig.at("server_name").end(); ++it)
			if (name == *it)
				return (true);
	return (false);
}

void ServerConfig::validateConfigValue(const std::string& fullContext, const std::vector<std::string>& values, const std::map<std::string, std::string>& fieldProperties) {
    validationFactory&	factory = validationFactory::getInstance();
    ValidationStrategy*	strategy = NULL;
    
    try {
        strategy = factory.getStrategy(fullContext);
        if (strategy) {
            for (std::vector<std::string>::const_iterator it = values.begin(); it != values.end(); ++it) {
                strategy->validate(*it, fieldProperties);
            }
        } else {
			log_message(Logger::ERROR, "No validation strategy found for context [%s]", fullContext.c_str());
			return ;
		}
    } catch (std::exception& e) {
        log_message(Logger::ERROR, "Error during context [%s], value [%s] validation: %s", fullContext.c_str(), values[0].c_str(), e.what());
        delete strategy;
        throw; 
    }
    
    delete strategy;
}
