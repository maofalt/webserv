/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: znogueir <znogueir@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/24 21:55:01 by rgarrigo          #+#    #+#             */
/*   Updated: 2023/09/25 01:36:09 by rgarrigo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include "HttpResponse.hpp"
#include "FDManager.hpp"

// Constructors
HttpResponse::HttpResponse(void):
	_port(0),
	_request(NULL),
	_method(""),
	_host(""),
	_uri(""),
	_path(""),
	_uriIsDirectory(false),
	_type(ERROR),
	_isSetUp(false),
	_iWriteToCgi(0),
	_pidCgi(0),
	_protocol(DEFAULT_PROTOCOL),
	_content(""),
	_raw(""),
	_iRaw(0)
{
}
HttpResponse::HttpResponse(uint16_t port):
	_port(port),
	_request(NULL),
	_method(""),
	_host(""),
	_uri(""),
	_path(""),
	_uriIsDirectory(false),
	_type(ERROR),
	_isSetUp(false),
	_iWriteToCgi(0),
	_pidCgi(0),
	_protocol(DEFAULT_PROTOCOL),
	_content(""),
	_raw(""),
	_iRaw(0)
{
}
HttpResponse::HttpResponse(HttpResponse const &rhs):
	_port(rhs._port),
	_request(rhs._request),
	_method(rhs._method),
	_uri(rhs._uri),
	_path(rhs._path),
	_uriIsDirectory(rhs._uriIsDirectory),
	_server(rhs._server),
	_location(rhs._location),
	_type(rhs._type),
	_isSetUp(rhs._isSetUp),
	_fdCgiIn(rhs._fdCgiIn),
	_iWriteToCgi(rhs._iWriteToCgi),
	_fdCgiOut(rhs._fdCgiOut),
	_pidCgi(rhs._pidCgi),
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
	if (_pidCgi != 0)
		kill(_pidCgi, SIGKILL);
}

// Operators
HttpResponse	&HttpResponse::operator=(HttpResponse const &rhs)
{
	_port = rhs._port;
	_request = rhs._request;
	_method = rhs._method;
	_uri = rhs._uri;
	_path = rhs._path;
	_uriIsDirectory = rhs._uriIsDirectory;
	_server = rhs._server;
	_location = rhs._location;
	_type = rhs._type;
	_isSetUp = rhs._isSetUp;
	_fdCgiIn = rhs._fdCgiIn;
	_iWriteToCgi = rhs._iWriteToCgi;
	_fdCgiOut = rhs._fdCgiOut;
	_pidCgi = rhs._pidCgi;
	_protocol = rhs._protocol;
	_status = rhs._status;
	_fields = rhs._fields;
	_content = rhs._content;
	_raw = rhs._raw;
	_iRaw = rhs._iRaw;

	return (*this);
}

void	HttpResponse::clean(void)
{
	_request = NULL;
	_method = "";
	_host = "";
	_uri = "";
	_path = "";
	_uriIsDirectory = false;
	_queryString = "";
	_server = NULL;
	_location = NULL;
	_fdsToClose.clear();
	_type = ERROR;
	_isSetUp = false;
	_fdCgiIn = 0;
	_iWriteToCgi = 0;
	_outputCgi = "";
	_pidCgi = 0;
	_envCgi.clear();
	_uploadFileOn = false;
	_uploadOnly = false;
	_cookie = "";
	_protocol = DEFAULT_PROTOCOL;
	_status = "";
	_fields.clear();
	_content = "";
	_raw = "";
	_iRaw = 0;
}

// Getters
int	HttpResponse::getFdCgiIn(void) const
{
	return (_fdCgiIn);
}
int	HttpResponse::getFdCgiOut(void) const
{
	return (_fdCgiOut);
}
bool	HttpResponse::isSetUp(void) const
{
	return (_isSetUp);
}

// Methods
void	HttpResponse::log(void) const
{
	log_message(Logger::DEBUG, "Response:");
	log_message(Logger::TRACE, "Response: %s", _raw.substr(0, 4096).c_str());
	if (_raw.size() > 4096)
		log_message(Logger::TRACE, "[...]");
}

int	HttpResponse::writeToCgi(void)
{
	long	bytesWritten;

	if (_method != "POST")
		return (0);
	_iWriteToCgi = 0;
	if (_request->_body.size() <= _iWriteToCgi + WRITE_CGI_BUFFER_SIZE)
		bytesWritten = write(_fdCgiIn, _request->_body.c_str() + _iWriteToCgi, _request->_body.size() - _iWriteToCgi);
	else
		bytesWritten = write(_fdCgiIn, _request->_body.c_str() + _iWriteToCgi, WRITE_CGI_BUFFER_SIZE);
	if (bytesWritten == -1)
		return (-1);
	_iWriteToCgi += bytesWritten;
	if (_iWriteToCgi >= _request->_body.size())
		return (0);
	return (1);
}

int	HttpResponse::readCgi(bool timeout)
{
	char	buffer[READ_BUFFER_SIZE] = {};
	int		bytesRead;

	if (timeout)
		return (kill(_pidCgi, SIGKILL), _pidCgi = 0, _writeError("504"));
	bytesRead = read(_fdCgiOut, buffer, READ_BUFFER_SIZE);
	if (bytesRead == 0)
		return (waitpid(_pidCgi, NULL, 0), _pidCgi = 0, _writeCgi());
	if (bytesRead == -1)
		return (waitpid(_pidCgi, NULL, 0), _pidCgi = 0, _writeError("500"));
	_outputCgi.append(buffer, bytesRead);
	return (bytesRead);
}

int	HttpResponse::send(int fd)
{
	std::string::size_type	bytesSend;

	if (_raw.size() <= _iRaw + SEND_BUFFER_SIZE)
		bytesSend = ::send(fd, _raw.c_str() + _iRaw, _raw.size() - _iRaw, 0);
	else
		bytesSend = ::send(fd, _raw.c_str() + _iRaw, SEND_BUFFER_SIZE, 0);
	if (bytesSend <= 0)
		return (-1);
	if (bytesSend == _raw.size() - _iRaw)
		return (0);
	_iRaw += bytesSend;
	return (1);
}

int	HttpResponse::setUp(HttpRequest const *request, Config &config)
{
	if (!request)
		return (_writeError("408"));
	_setRequest(request);
	if (_status != "200")
		return (_writeError(_status));
	_stripUri();
	if (_setServer(config)
		|| _limitClientBodySize()
		|| _determineLocation()
		|| _limitHttpMethod()
		|| _determinePost()
		|| _refineUri()
		|| _setType())
		return (_writeError(_status));
	log_message(Logger::DEBUG, "Response: _method = %s, no error detected!", _method.c_str());
	if (_type == CGI)
		return (_launchCgi());
	return ((this->*_writeType[_type])());
}
