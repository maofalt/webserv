/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: znogueir <znogueir@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/24 21:55:01 by rgarrigo          #+#    #+#             */
/*   Updated: 2023/09/02 02:41:17 by rgarrigo         ###   ########.fr       */
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
std::map<t_responseType, t_writeType>	HttpResponse::_getWriteType(void)
{
	std::map<t_responseType, t_writeType>	writeType;

	writeType[GET] = &HttpResponse::_writeGet;
	writeType[DELETE] = &HttpResponse::_writeDelete;
	writeType[DIRECTORY] = &HttpResponse::_writeDirectory;
	writeType[REDIRECTION] = &HttpResponse::_writeRedirection;
	return (writeType);
}
std::map<std::string, std::string>	HttpResponse::_description = getDescription();
std::map<std::string, std::string>	HttpResponse::_contentType = getContentType();
std::map<t_responseType, t_writeType>	HttpResponse::_writeType = HttpResponse::_getWriteType();

// Constructors
HttpResponse::HttpResponse(void):
	_uriIsDirectory(false),
	_type(ERROR),
	_protocol(DEFAULT_PROTOCOL),
	_raw(""),
	_iRaw(0)
{
}
HttpResponse::HttpResponse(uint16_t port):
	_port(port),
	_uriIsDirectory(false),
	_type(ERROR),
	_protocol(DEFAULT_PROTOCOL),
	_raw(""),
	_iRaw(0)
{
}
HttpResponse::HttpResponse(HttpResponse const &rhs):
	_port(rhs._port),
	_request(rhs._request),
	_method(rhs._method),
	_uri(rhs._uri),
	_uriIsDirectory(rhs._uriIsDirectory),
	_parameters(rhs._parameters),
	_server(rhs._server),
	_location(rhs._location),
	_type(rhs._type),
	_fdCgi(rhs._fdCgi),
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
	_uriIsDirectory = rhs._uriIsDirectory;
	_parameters = rhs._parameters;
	_server = rhs._server;
	_location = rhs._location;
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

int	HttpResponse::_setRequest(const HttpRequest *request)
{
	_request = request;
	_method = request->_method;
	_uri = request->_uri;
	_status = request->_status;
	return (0);
}

int	HttpResponse::_setServer(const Config &config)
{
	_server = config.findServer(_request->getHost(), _port);
	if (_server == 0)
		return (_status = "500", -1);
	return (0);
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
	while (_iRaw + SEND_BUFFER_SIZE <= _raw.size())
	{
		::send(fd, _raw.c_str() + _iRaw, SEND_BUFFER_SIZE, 0);
		_iRaw += SEND_BUFFER_SIZE;
	}
	return (::send(fd, _raw.c_str() + _iRaw, _raw.size() - _iRaw, 0));
//	int	ret_value;
//
//	if (_raw.size() <= _iRaw + SEND_BUFFER_SIZE)
//		return (::send(fd, _raw.c_str() + _iRaw, _raw.size() - _iRaw, 0));
//	ret_value = ::send(fd, _raw.c_str() + _iRaw, SEND_BUFFER_SIZE, 0);
//	_iRaw += SEND_BUFFER_SIZE;
//	return (ret_value);
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
		std::getline(ss, stripedUri, '/');
		if (_host.empty())
			_host = stripedUri;
		std::getline(ss, stripedUri, '\0');
		_uri = std::string("/");
		_uri += stripedUri;
	}
	return (0);
}

int	HttpResponse::_limitClientBodySize(void)
{
	if (!_server->_maxSize && _request->_body.size() > _server->_maxSize)
		return (_status = "413", -1);
	return (0);
}

int	HttpResponse::_limitHttpMethod(void)
{
	if (_location->_locConfig.count("methods"))
		for (std::vector<std::string>::const_iterator it = _location->_locConfig.at("methods").begin(); it != _location->_locConfig.at("methods").end(); ++it)
			if (*it == _method)
				return (0);
	return (_status = "405", -1);
}

int	HttpResponse::_determineLocation(void)
{
	std::string::size_type	maxLen = 0;

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
	return (0);
}

int	HttpResponse::_refineUri(void)
{
	std::string::size_type	maxLen = 0;
	struct stat				statbuf;

	if (!_location->_locConfig.count("root"))
		return (0);
	for (std::vector<std::string>::const_iterator prefix = _location->_paths.begin(); prefix != _location->_paths.end(); ++prefix)
		if (_uri.find(*prefix) == 0 && prefix->size() > maxLen)
			maxLen = prefix->size();
	_uri.replace(0, maxLen, _location->_locConfig.at("root")[1]);
	if (access(_uri.c_str(), F_OK) == -1)
		return (_status = "404", -1);
	if (access(_uri.c_str(), R_OK) == -1)
		return (_status = "403", -1);
	if (stat(_uri.c_str(), &statbuf) == -1)
		return (_status = "500", -1);
	if (statbuf.st_mode & S_IFDIR)
	{
		if (*_uri.end() != '/')
			_uri.push_back('/');
		if (_location->_locConfig.count("index"))
			_uri += _location->_locConfig.at("index")[1];
		else
			_uriIsDirectory = true;
	}
	return (0);
}

int	HttpResponse::_setType(void)
{
	if (_method == "GET")
		_type = GET;
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

int	HttpResponse::_launchCgi(void)
{
	return (_writeError("500"));
}
int	HttpResponse::_writeRedirection(void)
{
	return (_writeError("500"));
}
int	HttpResponse::_writeDirectory(void)
{
	return (_writeError("500"));
}
int	HttpResponse::_writeCgi(void)
{
	return (_writeError("500"));
}
int	HttpResponse::_writeGet(void)
{
	std::stringstream	buffer;
	std::ostringstream	oss;


	log_message(Logger::DEBUG, "Request:");
	oss << *_request << std::endl;
	log_message(Logger::TRACE, "Request: %s", oss.str().c_str());
	

	if (_uri == "/")
		_uri = "/index.html";

	std::ifstream	file(_uri.c_str());
	if (!file.is_open())
	{
		ERROR_LOG("File not opened");
		_uri = "./site2/errors/404.html";
		_status = "404";
		std::ifstream	file2(_uri.c_str());
		if (!file2.is_open())
			return (1);
		log_message(Logger::WARN, "404 opened");
		buffer << file2.rdbuf();
	}
	buffer << file.rdbuf();
	file.close();
	_content = buffer.str();

	return (_writeRaw());
}
int	HttpResponse::_writeDelete(void)
{
	return (_writeError("500"));
}
int	HttpResponse::_writeError(std::string status)
{
	_status = status;
	_content = _defaultErrorPages[status];
	return (_writeRaw());
}
int	HttpResponse::_writeRaw(void)
{
	std::string			extension;

	// we build the _raw
	// 1- Status line HTTP-Version Status-Code Reason-Phrase CRLF

	_raw += _protocol;
	_raw += " ";
	_raw += _status;
	_raw += " ";
	_raw += _description[_status];
	_raw += "\r\n";

// 2- Date: Date and time of the message CRLF
	char	time_buffer[1000];
	time_t	now = time(0);
	struct tm	tm = *gmtime(&now);
	strftime(time_buffer, 1000, "%a, %d %b %Y %H:%M:%S %Z", &tm);
	_raw += "Date: ";
	_raw += time_buffer;
	_raw += "\r\n";

// 3- Server: Information about the server CRLF
	_raw += "Server: ";
	_raw += "Webserv";
	_raw += "\r\n";

// 4- Content-Type: Type of the message body CRLF
	if (_status == "200")
		extension = _uri.substr(_uri.find_last_of(".") + 1);
	else
		extension = "html";
	_raw += "Content-Type: ";
	_raw += _contentType[extension];
	_raw += "\r\n";

// 5- Content-Length: Size of the message body in bytes CRLF
	_raw += "Content-Length: ";
	_raw += numberToString(_content.size());
	_raw += "\r\n";

	_raw += "Connection: ";
	_raw += "keep-alive";
	_raw += "\r\n";

	_raw += "Accept-Ranges: ";
	_raw += "bytes";
	_raw += "\r\n";

	_raw += "\r\n";

	_raw += _content;

//	log_message(Logger::DEBUG, "Response:");
//	if (extension == "html" || extension == "css")
//	{
//		log_message(Logger::TRACE, "Response: %s", _raw.substr(0, 4096).c_str());
//		if (_raw.size() > 4096)
//			log_message(Logger::TRACE, "[...]");
//	}
//	else
//		log_message(Logger::TRACE, "File \"%s\" not printable", _uri.c_str());

	return (0);
}

int	HttpResponse::setUp(HttpRequest const *request, const Config &config)
{
	_setRequest(request);
	_stripUri();
	if (_setServer(config)
		|| _limitClientBodySize()
		|| _determineLocation()
		|| _limitHttpMethod()
		|| _refineUri()
		|| _setType())
		return (_writeError(_status));
	if (_type == CGI)
		return (_launchCgi());
	return ((this->*_writeType[_type])());
}
