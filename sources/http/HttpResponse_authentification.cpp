/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse_authentification.cpp                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgarrigo <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/25 01:30:18 by rgarrigo          #+#    #+#             */
/*   Updated: 2023/09/25 02:16:06 by rgarrigo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpResponse.hpp"

std::string	HttpResponse::_generateCookie(void)
{
	std::string	allowedCharacters(
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"_-");
	std::string	cookie;

	for (int i = 0; i < 30; ++i)
		cookie.push_back(allowedCharacters[std::rand() % allowedCharacters.size()]);
	return (cookie);
}

int	HttpResponse::_getAuthentification(std::string &user, std::string &password, std::vector<std::string> &rolesRequested)
{
	std::istringstream			ss(_request->_body);
	std::string					name;
	std::string					value;
	std::string					role;

	while (!ss.eof())
	{
		std::getline(ss, name, '=');
		std::getline(ss, value, '&');
		if (name == "user" && user == "")
			user = value;
		if (name == "password" && password == "")
			password = value;
		if (name == "role")
		{
			std::istringstream			ssValue(value);

			while (!ssValue.eof())
			{
				std::getline(ssValue, role, ',');
				rolesRequested.push_back(role);
			}
		}
	}
	return (0);
}

int	HttpResponse::_checkAuthentification(const std::string &user, const std::string &password, const std::vector<std::string> &rolesRequested, std::vector<std::string> &rolesAccepted)
{
	if (user == ""
		|| password == ""
		|| rolesRequested.size() == 0)
		return (2);
	if (!_server->_credentials.count(user))
		return (3);
	for (std::vector<std::string>::const_iterator role = rolesRequested.begin(); role != rolesRequested.end(); ++role)
	{
		if (!_server->_credentials.at(user).count(*role))
			continue ;
		if (_server->_credentials.at(user).at(*role) == password)
			rolesAccepted.push_back(*role);
	}
	if (rolesAccepted.size() == 0)
		return (3);
	return (0);
}

int	HttpResponse::_setAuthentificationCookie(const std::string &user, const std::vector<std::string> &rolesAccepted)
{
	_cookie = _generateCookie();
	_server->_sessionCookie[_cookie]["user"] = user;
	for (std::vector<std::string>::const_iterator role = rolesAccepted.begin(); role != rolesAccepted.end(); ++role)
		_server->_sessionCookie[_cookie][*role] = "on";
	return (0);
}

int	HttpResponse::_authentificate(void)
{
	std::string					user;
	std::string					password;
	std::vector<std::string>	rolesRequested;
	std::vector<std::string>	rolesAccepted;
	int							status;

	if (!_request->_field.count("Content-Type")
		|| _request->_field.at("Content-Type") != "application/x-www-form-urlencoded")
		return (2);
	_getAuthentification(user, password, rolesRequested);
	status = _checkAuthentification(user, password, rolesRequested, rolesAccepted);
	if (!status)
		_setAuthentificationCookie(user, rolesAccepted);
	return (status);
}
