/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse_set.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgarrigo <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/25 01:33:35 by rgarrigo          #+#    #+#             */
/*   Updated: 2023/09/25 01:34:17 by rgarrigo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpResponse.hpp"

int	HttpResponse::_setRequest(const HttpRequest *request)
{
	_request = request;
	_method = request->_method;
	_uri = request->_uri;
	_status = request->_status;
	return (0);
}

int	HttpResponse::_setServer(Config &config)
{
	log_message(Logger::DEBUG, "Setting server in the response...");
	log_message(Logger::DEBUG, "Host: %s", _request->getHost().c_str());
	log_message(Logger::DEBUG, "Port: %d", _port);
	_server = config.findServer(_request->getHost(), _port);
	if (_server == 0)
		return (_status = "500", -1);
	return (0);
}
int	HttpResponse::_determinePost(void)
{
	_uploadFileOn = _location->_locConfig.count("upload")
		&& _method == "POST"
		&& _request->_field.count("Content-Type")
		&& _request->_field.at("Content-Type").find("multipart/form-data") == 0;
	_uploadOnly = (_uploadFileOn && !_location->_locConfig.count("cgi"))
		|| (_method == "POST" && _uri == "/login");
	return (0);
}

int	HttpResponse::_stripUri(void)
{
	std::istringstream	ss(_uri);

	std::getline(ss, _uri, '?');
	std::getline(ss, _queryString, '\0');
	return (0);
}

int	HttpResponse::_limitClientBodySize(void)
{
	if (_server->_servConfig.count("clientBodyLimit") && _request->_body.size() > (std::string::size_type)std::atoi(_server->_servConfig.at("clientBodyLimit")[1].c_str()))
		return (_status = "413", -1);
	return (0);
}

int	HttpResponse::_limitHttpMethod(void)
{
	if (_location->_locConfig.count("return"))
		return (0);
	if (_location->_locConfig.count("methods"))
		for (std::vector<std::string>::const_iterator it = _location->_locConfig.at("methods").begin(); it != _location->_locConfig.at("methods").end(); ++it)
			if (*it == _method)
				return (0);
	return (_status = "405", -1);
}

bool	HttpResponse::_locationAllowed(void)
{
	std::string				cookie;
	std::string::size_type	i;
	std::string::size_type	j;

	if (!_location->_locConfig.count("allow"))
		return (true);
	if (!_request->_field.count("Cookie"))
		return (false);
	i = 0;
	while (_request->_field.at("Cookie").find("authentification=", i) != i)
	{
		i = _request->_field.at("Cookie").find("; ", i);
		if (i == std::string::npos)
			return (false);
		i += 2;
	}
	i += 17;
	j = _request->_field.at("Cookie").find(";", i);
	cookie = _request->_field.at("Cookie").substr(i, j);
	if (!_server->_sessionCookie.count(cookie))
		return (false);
	for (std::vector<std::string>::const_iterator role = ++_location->_locConfig.at("allow").begin(); role != _location->_locConfig.at("allow").end(); ++role)
		if (_server->_sessionCookie.at(cookie).count(*role))
			return (true);
	return (false);
}
int	HttpResponse::_determineLocation(void)
{
	std::string::size_type	maxLen = 0;

	if (_uri.find("/css") == 0
		&& _uri.find("/css/dark") != 0
		&& _request->_field.count("Cookie")
		&& _request->_field.at("Cookie").find("darkmode=on") != std::string::npos)
		_uri.replace(0, 4, "/css/dark");
	for (std::vector<t_location>::const_iterator location = _server->_locations.begin(); location != _server->_locations.end(); ++location)
	{
		for (std::vector<std::string>::const_iterator prefix = location->_paths.begin(); prefix != location->_paths.end(); ++prefix)
		{
			if (_uri.find(*prefix) == 0 && prefix->size() > maxLen)
			{
				maxLen = prefix->size();
				_location = &(*location);
			}
		}
	}
	if (maxLen == 0)
		return (_status = "404", -1);
	if (!_locationAllowed())
		return (_status = "401", -1);
	return (0);
}

int	HttpResponse::_checkPath(void)
{
	std::string	index;
	struct stat	statbuf;

	if (access(_path.c_str(), F_OK) == -1)
		return (_status = "404", -1);
	if (access(_path.c_str(), R_OK) == -1)
		return (_status = "403", -1);
	if (stat(_path.c_str(), &statbuf) == -1)
		return (_status = "500", -1);
	if (statbuf.st_mode & S_IFDIR)
	{
		if (*_path.rbegin() != '/')
			_path.push_back('/');
		if (*_uri.rbegin() != '/')
			_uri.push_back('/');
		if (_location->_locConfig.count("index"))
		{
			index = _path;
			index += _location->_locConfig.at("index")[1];
			if (access(index.c_str(), F_OK) == -1)
				_uriIsDirectory = true;
			else
			{
				if (access(index.c_str(), R_OK) == -1)
					return (_status = "403", -1);
				_path = index;
			}
		}
		else
			_uriIsDirectory = true;
	}
	if (_uriIsDirectory
		&& _location->_locConfig.count("directoryListing")
		&& _location->_locConfig.at("directoryListing")[1] == "off")
		return (_status = "403", -1);
	return (0);
}
int	HttpResponse::_refineUri(void)
{
	std::string::size_type	maxLen = 0;

	if (_uploadOnly)
		return (0);
	if (!_location->_locConfig.count("root"))
		return (0);
	for (std::vector<std::string>::const_iterator prefix = _location->_paths.begin(); prefix != _location->_paths.end(); ++prefix)
		if (_uri.find(*prefix) == 0 && prefix->size() > maxLen)
			maxLen = prefix->size();
	_path = _uri;
	_path.replace(0, maxLen, _location->_locConfig.at("root")[1]);
	return (_checkPath());
}

int	HttpResponse::_setType(void)
{
	if (_method == "GET")
		_type = GET;
	if (_uploadOnly)
		_type = AUTHENTIFICATION;
	if (_uploadFileOn)
		_type = UPLOAD;
	if (_location->_locConfig.count("cgi"))
		_type = CGI;
	if (_uriIsDirectory)
		_type = DIRECTORY;
	if (_method == "DELETE")
		_type = DELETE;
	if (_location->_locConfig.count("return"))
		_type = REDIRECTION;
	return (0);
}
