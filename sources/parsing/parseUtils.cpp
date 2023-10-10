/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parseUtils.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: znogueir <znogueir@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/28 15:03:21 by znogueir          #+#    #+#             */
/*   Updated: 2023/08/28 15:03:22 by znogueir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"

void	Config::checkRetToL(std::vector<std::string>::iterator & it,
		int & countLines) {

	countLines++;
	if (it != _splitContent.begin() && *it == "\n" && *(it - 1) != ";"  && *(it - 1) != "\n" \
			&& *(it - 1) != "{"  && *(it - 1) != "}") {
		printErr("expected ';' before '\\n'.", countLines - 1);
		_nbrErr++;
	}
}

void	Config::checkOpenBrack(std::vector<std::string>::iterator & it,
		int & countLines, int & bracketOpen) {

	if (it + 1 != _splitContent.end() && *(it + 1) != "\n") {
		printErr("expected '\\n' after '{'.", countLines);
		_nbrErr++;
	}
	bracketOpen++;
}

void	Config::checkCloseBrack(std::vector<std::string>::iterator & it,
		int & countLines, int & bracketOpen) {

	if (it + 1 != _splitContent.end() && *(it + 1) != "\n") {
		printErr("expected '\\n' after '}'.", countLines);
		_nbrErr++;
	}
	else if (it != _splitContent.begin() && *(it - 1) != "\n") {
		printErr("expected '\\n' before '}'.", countLines);
		_nbrErr++;
	}
	bracketOpen--;
}

void	Config::checkSemiCol(std::vector<std::string>::iterator & it,
		int & countLines) {

	if ((it + 1) != _splitContent.end() && *(it + 1) != "\n") {
		printErr("expected '\\n' after ';'.", countLines);
		_nbrErr++;
	}
}