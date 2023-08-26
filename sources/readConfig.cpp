/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   readConfig.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: znogueir <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/26 18:16:43 by znogueir          #+#    #+#             */
/*   Updated: 2023/08/26 18:16:44 by znogueir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"

void	removeComments(std::string &line) {
	size_t	pos = line.find_first_of('#');

	if (pos != std::string::npos)
		line.erase(pos, line.size() - pos);
}

void	Config::readConf(std::ifstream & file) {
	std::string	line;

	while (file)
	{
		std::getline(file, line);
		removeComments(line);
		_rawContent.push_back(line + "\n");
	}
	file.close();
}

void	Config::splitConf() {
	size_t	size = _rawContent.size();
	size_t	start;
	size_t	end;

	for (size_t i=0; i<size; i++) {
		while (!_rawContent[i].empty()) {
			start = _rawContent[i].find_first_not_of(_tokens);
			if (start != std::string::npos) {
				end = _rawContent[i].find_first_of(_tokens, start);
				if (end != std::string::npos) {
					_splitContent.push_back(_rawContent[i].substr(start, end - start));
					while (_tokens.find(_rawContent[i][end]) != std::string::npos) {
						if (_whiteSp.find(_rawContent[i][end]) == std::string::npos)
							_splitContent.push_back(_rawContent[i].substr(end, 1));
						end++;
					}
				}
			}
			else {
				int	j = 0;
				while (_tokens.find(_rawContent[i][j]) != std::string::npos) {
					if (_whiteSp.find(_rawContent[i][j]) == std::string::npos)
						_splitContent.push_back(_rawContent[i].substr(j, 1));
					j++;
				}
			}
			_rawContent[i].erase(0, end);
		}
	}
}
