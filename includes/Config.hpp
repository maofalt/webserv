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
		std::vector<std::string>	_rawContent;
		std::vector<std::string>	_splitContent;
		std::string					_tokens;
		std::string					_whiteSp;
		// vector

	public:
		Config(/* args */);
		~Config();

		static bool					checkNorm(std::ifstream & file);
		void						readConf(std::ifstream & file);
		void						splitSemiCol();
		void						rmWhiteSpaces();
		void						splitConf();
		bool						setupConf(std::ifstream & file);
		std::vector<std::string>	getRawContent() const;
		std::vector<std::string>	getSplitContent() const;
};

std::ostream& operator<<(std::ostream& os, const Config & conf);

#endif
