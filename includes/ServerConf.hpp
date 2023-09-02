/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConf.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: znogueir <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/26 16:47:00 by znogueir          #+#    #+#             */
/*   Updated: 2023/09/02 01:21:07 by rgarrigo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef	SERVERCONF_HPP
# define SERVERCONF_HPP

# include <iostream>
# include <map>
# include <set>
# include <vector>

typedef struct location {
	std::vector< std::string >							_paths;
	std::map< std::string, std::vector< std::string > >	_locConfig;
}	t_location;

class ServerConfig {
	public:
		std::map< std::string, std::vector< std::string > >	_servConfig;
		std::vector< struct location >						_locations;
		std::string::size_type								_maxSize;
		std::set<std::string>								_allowedMethods;

		bool	isListeningTo(uint16_t port) const;
		bool	isNamed(const std::string &name) const;
};

#endif
