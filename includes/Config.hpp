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

#include <vector>
#include <fstream>
#include <iostream>

class Config {
	private:
		std::vector<std::string>	rawContent;
		std::vector<std::string>	splitContent;
		// vector

	public:
		Config(/* args */);
		~Config();

		static bool					checkNorm(std::ifstream & file);
		void						readConf(std::ifstream & file);
		void						splitSemiCol();
		void						splitConf();
		bool						setupConf(std::ifstream & file);
		std::vector<std::string>	getRawContent() const {return this->rawContent;};
		std::vector<std::string>	getSplitContent() const {return this->splitContent;};
};

std::ostream& operator<<(std::ostream& os, const Config & conf);

#endif
