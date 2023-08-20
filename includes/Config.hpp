/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: znogueir <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/20 14:42:02 by znogueir          #+#    #+#             */
/*   Updated: 2023/08/20 14:42:05 by znogueir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
# define CONFIG_HPP

#include <fstream>
#include <iostream>

class Config {
	private:
		std::string	rawContent;
		// vector

	public:
		Config(/* args */);
		~Config();

		static bool	checkNorm(std::ifstream & file);
		void		readConf(std::ifstream & file);
		void		setupConf(std::ifstream & file);
};

#endif
