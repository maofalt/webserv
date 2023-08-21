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
		std::string					_confFileName;
		// vector

	public:
		// missing coplien !!!!;
		Config();
		~Config();

		void						readConf(std::ifstream & file);
		void						splitConf();
		bool						basicCheck();
		bool						setupConf(std::ifstream & file, std::string fileName);
		std::vector<std::string>	getRawContent() const;
		std::vector<std::string>	getSplitContent() const;
};

std::ostream& operator<<(std::ostream& os, const Config & conf);

#endif
