/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgarrigo <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/24 21:55:01 by rgarrigo          #+#    #+#             */
/*   Updated: 2023/08/26 19:25:12 by rgarrigo         ###   ########.fr       */
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
	_uriIsDirectory(false),
	_type(ERROR),
	_protocol(DEFAULT_PROTOCOL),
	_iRaw(0)
{
}
HttpResponse::HttpResponse(uint16_t port):
	_port(port),
	_uriIsDirectory(false),
	_type(ERROR),
	_protocol(DEFAULT_PROTOCOL),
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
		_uri += stripUri;
	}
	return (0);
}

int	HttpResponse::_limitClientBodySize(void)
{
	if (_request->body.size() > _server->maxSize)
		return (_writeError("413"));
	return (0);
}

int	HttpResponse::_limitHttpMethod(void)
{
	if (_server->allowedMethods.count(_method) == 0)
		return (_writeError("405"));
	return (0);
}

int	HttpResponse::_determineLocation(void)
{
	std::string::size_type	maxLen = 0;

	for (std::vector<t_location>::iterator location = _server->_locations.begin(); location != _server->_locations.end(); ++location)
	{
		for (std::vector<std::string>::iterator prefix = location->_paths.begin(); prefix != location->_paths.end(); ++prefix)
		{
			if (_uri.find(*prefix) == 0 && prefix->size() > maxLen)
			{
				maxLen = prefix->size();
				_location = &(*location);
			}
		}
	}
	if (maxLen == 0)
		return (_writeError("404"));
	return (0);
}

int	HttpResponse::_refineUri(void)
{
	std::string::size_type	maxLen = 0;
	struct stat				statbuf;

	if (!_location->locConfig.count("root"))
		return (0);
	for (std::vector<std::string>::iterator prefix = _location->_paths.begin(); prefix != _location->_paths.end(); ++prefix)
		if (_uri.find(*prefix) == 0 && prefix->size() > maxLen)
			maxLen = prefix->size();
	_uri.replace(0, maxLen, _location->locConfig.at("root"));
	if (access(_uri.c_str(), F_OK) == -1)
		return (_writeError("404"));
	if (access(_uri.c_str(), R_OK) == -1)
		return (_writeError("403"));
	if (stat(_uri.c_str(), &statbuf) == -1)
		return (_writeError("500"));
	if (statbuf.st_mode & S_IFDIR)
	{
		if (_uri.back() != '/')
			_uri.push_back('/');
		if (_location->locConfig.count("index"))
			_uri += _location->locConfig.at("index");
		else
			_uriIsDirectory = true;
	}
	return (0);
}

int	HttpResponse::_setType(void)
{
	if (_method == "GET")
		_type = GET;
	if (_location->locConfig.count("cgi"))
		_type = CGI;
	if (_uriIsDirectory)
		_type = DIRECTORY;
	if (_method == "DELETE")
		_type = DELETE;
	if (_location->locConfig.count("return"))
		_type = REDIRECTION;
	return (0);
}

int	HttpResponse::_launchCgi(void)
{
	return (_writeError("500"));
}

int	HttpResponse::setUp(HttpRequest const *request, const Config &config)
{
	_setRequest(request);
	_stripUri();
	_setServer(config);
	if (_limitClientBodySize()
		|| _limitHttpMethod()
		|| _determineLocation()
		|| _refineUri()
		|| _setType())
		return (-1);
	if (_type == CGI)
		return (_launchCgi());
	return (_writeType[_type]);
}

int	HttpResponse::respond(int fd, std::string status)
{
	std::string			response;
	std::stringstream	buffer;
	std::string			extension;

	std::cout << "\033[31mRequest:\033[0m" << std::endl;
	std::cout << *_request << std::endl;

	if (_uri == "/")
		_uri = "/index.html";

	// we try to open the requested page and if it fails we send a 404 error
	_uri = "./site" + _uri;
	std::ifstream	file(_uri.c_str());
	if (!file.is_open())
	{
		std::cerr << "File not opened" << std::endl;
		_uri = "./site/errors/404.html";
		status = "404";
		std::ifstream	file2(_uri.c_str());
		if (!file2.is_open())
			return (1);
		std::cout << "404 opened" << std::endl;
		buffer << file2.rdbuf();
	}
	buffer << file.rdbuf();
	file.close();
	_content = buffer.str();

	// we build the response
	// 1- Status line HTTP-Version Status-Code Reason-Phrase CRLF
	
	response += _protocol;
	response += " ";
	response += status;
	response += " ";
	response += _description[status];
	response += "\r\n";

// 2- Date: Date and time of the message CRLF
	char	time_buffer[1000];
	time_t	now = time(0);
	struct tm	tm = *gmtime(&now);
	strftime(time_buffer, 1000, "%a, %d %b %Y %H:%M:%S %Z", &tm);
	response += "Date: ";
	response += time_buffer;
	response += "\r\n";

