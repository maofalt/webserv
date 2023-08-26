/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgarrigo <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/24 21:55:01 by rgarrigo          #+#    #+#             */
/*   Updated: 2023/08/26 17:37:38 by rgarrigo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include "HttpResponse.hpp"

// Static member variables
static std::map<std::string, std::string>	getDescription(void)
{
	std::map<std::string, std::string>	description;

	description["200"] = "OK";
	description["400"] = "Bad request";
	description["403"] = "Forbidden";
	description["404"] = "Not found";
	description["405"] = "Method not allowed";
	description["411"] = "Length required";
	description["413"] = "Request entity too large";
	description["500"] = "Internal server error";
	description["501"] = "Not implemented";
	description["505"] = "HTTP Version not supported";
	return (description);
}
static std::map<std::string, std::string>	getContentType(void)
{
	std::map<std::string, std::string>	contentType;

	contentType["html"] = "text/html";
	contentType["css"] = "text/css";
	contentType["jpg"] = "image/jpeg";
	contentType["jpeg"] = "image/jpeg";
	contentType["png"] = "image/png";
	return (contentType);
}
static std::map<t_responseType, t_writeType>	getWriteType(void)
{
	std::map<t_responseType, t_writeType>	writeType;

	writeType[GET] = _writeGet;
	writeType[DELETE] = _writeDelete;
	writeType[DIRECTORY] = _writeDirectory;
	writeType[REDIRECTION] = _writeRedirection;
	return (writeType);
}
std::map<std::string, std::string>	HttpResponse::_description = getDescription();
std::map<std::string, std::string>	HttpResponse::_contentType = getContentType();
std::map<std::string, std::string>	HttpResponse::_writeType = getWriteType();

// Constructors
HttpResponse::HttpResponse(void):
	_protocol(DEFAULT_PROTOCOL),
	_iRaw(0)
{
}
HttpResponse::HttpResponse(uint16_t port):
	_port(port),
	_protocol(DEFAULT_PROTOCOL),
	_iRaw(0)
{
}
HttpResponse::HttpResponse(HttpResponse const &rhs):
	_port(rhs._port),
	_request(rhs._request),
	_method(rhs._method),
	_uri(rhs._uri),
	_parameters(rhs._parameters),
	_server(rhs._server),
	_protocol(rhs._protocol),
	_status(rhs._status),
	_fields(rhs._fields),
	_content(rhs._content),
	_raw(rhs._raw),
	_iRaw(rhs._iRaw)
{
}
HttpResponse::HttpResponse(HttpRequest const *request):
	_request(request)
{
}

// Destructor
HttpResponse::~HttpResponse(void)
{
}

// Operators
HttpResponse	&HttpResponse::operator=(HttpResponse const &rhs)
{
	_port = rhs._port;
	_request = rhs._request;
	_method = rhs._method;
	_uri = rhs._uri;
	_parameters = rhs._parameters;
	_server = rhs._server;
	_type = rhs._type;
	_fdCgi = rhs._fdCgi;
	_protocol = rhs._protocol;
	_status = rhs._status;
	_fields = rhs._fields;
	_content = rhs._content;
	_raw = rhs._raw;
	_iRaw = rhs._iRaw;

	return (*this);
}

// Utils
template <class T>
std::string	numberToString(T nb)
{
	std::ostringstream	ss;
	ss << nb;
	return (ss.str());
}

void	HttpResponse::_setRequest(HttpRequest const *request)
{
	_request = request;
	_method = request->_method;
	_uri = request->_uri;
	_status = request->_status;
}

void	HttpResponse::_setServer(const Config &config)
{
	_server = config.findServer(_request->getHost(), _port);
}

// Methods
int	HttpResponse::readCgi(bool timeout)
{
	char	buffer[READ_BUFFER_SIZE] = {};
	int		bytesRead;

	if (timeout)
		return (close(_fdCgi), _writeError("504"));
	bytesRead = read(_fdCgi, buffer, READ_BUFFER_SIZE);
	if (bytesRead == -1)
		return (close(_fdCgi), _writeError("500"));
	_content.append(buffer, bytesRead);
	if (bytesRead == 0)
		return (_writeCgi());
	return (bytesRead);
}

int	HttpResponse::send(int fd)
{
	int	ret_value;

	if (_iRaw + SEND_BUFFER_SIZE <= _raw.size())
		return (::send(fd, _raw.c_str() + _iRaw, _raw.size() - _iRaw, 0));
	ret_value = ::send(fd, _raw.c_str() + _iRaw, SEND_BUFFER_SIZE, 0);
	_iRaw += SEND_BUFFER_SIZE;
	return (ret_value);
}

int	HttpResponse::_limitClientBodySize(void)
{
	if (_request->body.size() > _server->maxSize)
		return (_writeError("413"));
	return (0);
}

int	HttpResponse::_stripUri(void)
{
	std::stringstream	ss(_uri);
	std::string			stripedUri;
	std::string			name;
	std::string			value;

	std::getline(ss, stripedUri, ':');
	std::getline(ss, stripedUri, '?');
	while (std::getline(ss, name, '='))
	{
		if (!std::getline(ss, value, '&'))
			break ;
		_parameters[name] = value;
	}
	_uri = stripedUri;
	if (_uri.size() >= 2 && _uri[0] == '/' && _uri[1] == '/')
	{
		std::stringstream	ss(_uri);

		ss.ignore(2);
		std::getline(ss, _host, '/');
		std::getline(ss, stripedUri, '\0');
		_uri = std::string("/");
		_uri += stripUri;
	}
	return (0);
}

int	HttpResponse::_limitHttpMethod(void)
{
	
}

int	HttpResponse::setUp(HttpRequest const *request, const Config &config)
{
	_setRequest(request);
	_setServer(config);
	if (_limitClientBodySize()
		|| _stripUri()
		|| _limitHttpMethod()
		|| _refineUri()
		|| _setType())
		return (-1);
	if (_type == CGI)
		return (_launchCgi());
	return (_writeType[_type]);
}
