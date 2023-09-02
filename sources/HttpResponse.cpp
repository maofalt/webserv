/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: znogueir <znogueir@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/24 21:55:01 by rgarrigo          #+#    #+#             */
/*   Updated: 2023/09/02 23:52:22 by rgarrigo         ###   ########.fr       */
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
std::map<std::string, std::string>	HttpResponse::_mapContentType = getContentType();
std::map<t_responseType, t_writeType>	HttpResponse::_writeType = HttpResponse::_getWriteType();

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
	_protocol(DEFAULT_PROTOCOL),
	_contentType(""),
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
	_protocol(DEFAULT_PROTOCOL),
	_contentType(""),
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
	_parameters(rhs._parameters),
	_server(rhs._server),
	_location(rhs._location),
	_type(rhs._type),
	_fdCgi(rhs._fdCgi),
	_protocol(rhs._protocol),
	_status(rhs._status),
	_fields(rhs._fields),
	_content(rhs._content),
	_contentType(rhs._contentType),
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
	_path = rhs._path;
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
	_contentType = rhs._contentType;
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
	if (_location->_locConfig.count("return"))
		return (0);
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
	std::string				index;

	if (!_location->_locConfig.count("root"))
		return (0);
	for (std::vector<std::string>::const_iterator prefix = _location->_paths.begin(); prefix != _location->_paths.end(); ++prefix)
		if (_uri.find(*prefix) == 0 && prefix->size() > maxLen)
			maxLen = prefix->size();
	_path = _uri;
	_path.replace(0, maxLen, _location->_locConfig.at("root")[1]);
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
	std::ostringstream	content;

	_status = "301";
	_fields["Location"] = _location->_locConfig.at("return")[1];
	content << "<!DOCTYPE html>\n"
		<< "<html lang=\"en\">\n"
		<< "<head>\n"
		<< "	<meta charset=\"utf-8\" />\n"
		<< "	<title>301 - Moved</title>\n"
		<< "</head>\n"
		<< "<body>\n"
		<< "	<p>This page has moved to <a href=\"" << _fields["Location"]
		<< "\">" << _fields["Location"] << "</a>.</p>\n"
		<< "</body>\n"
		<< "</html>\n";
	_content = content.str();
	return (_writeRaw());
}
int	HttpResponse::_writeDirectory(void)
{
	std::ostringstream	content;
	DIR					*dir;
	struct dirent		*entry;

	content << "<!DOCTYPE html>\n"
			<< "<html lang=\"en\">\n"
			<< "<head>\n"
			<< "	<meta charset=\"utf-8\" />\n"
			<< "	<title>"<< _uri << "</title>\n"
			<< "</head>\n"
			<< "<body>\n"
			<< "	<h1>" << _uri << "</h1>\n"
			<< "	<ul>\n";
	dir = opendir(_path.c_str());
	entry = readdir(dir);
	while (entry)
	{
		content << "		<li><a href=\"" << _uri << entry->d_name << "\">" << entry->d_name << "</a></li\n>";
		entry = readdir(dir);
	}
	closedir(dir);
	content	<< "	</ul>\n"
			<< "</body>\n"
			<< "</html>\n";
	_content = content.str();
	return (_writeRaw());
}
int	HttpResponse::_writeCgi(void)
{
	return (_writeError("500"));
}
int	HttpResponse::_writeGet(void)
{
	std::stringstream	buffer;
	std::ifstream		file(_path.c_str());
	std::string			extension;

	if (!file.is_open())
		return (ERROR_LOG("File not opened"), _writeError("404"));
	buffer << file.rdbuf();
	file.close();
	_content = buffer.str();

	extension = _path.substr(_path.find_last_of(".") + 1);
	_contentType = _mapContentType[extension];

	return (_writeRaw());
}
int	HttpResponse::_writeDelete(void)
{
	std::ostringstream	content;

	if (std::remove(_path.c_str()))
		return (_writeError("500"));
	content << "<!DOCTYPE html>\n"
		<< "<html lang=\"en\">\n"
		<< "<head>\n"
		<< "	<meta charset=\"utf-8\" />\n"
		<< "	<title>Deleted</title>\n"
		<< "</head>\n"
		<< "<body>\n"
		<< "	<p>File has been deleted.</p>\n"
		<< "</body>\n"
		<< "</html>\n";
	_content = content.str();
	return (_writeRaw());
}
int	HttpResponse::_writeError(std::string status)
{
	_status = status;
	_content = _defaultErrorPages[status];
	return (_writeRaw());
}
int	HttpResponse::_writeRaw(void)
{
	std::ostringstream	response;

	char	time_buffer[1000];
	time_t	now = time(0);
	struct tm	tm = *gmtime(&now);
	strftime(time_buffer, 1000, "%a, %d %b %Y %H:%M:%S %Z", &tm);

	response << _protocol << " " << _status << " " << _description[_status] << "\r\n";
	response << "Date: " << time_buffer << "\r\n";
	response << "Server: " << "webserv" << "\r\n";
	response << "Content-Type: " << _contentType << "\r\n";
	response << "Content-Length: " << numberToString(_content.size()) << "\r\n";
	response << "Connection: " << "keep-alive" << "\r\n";
	response << "Accept-Ranges: " << "bytes" << "\r\n";
	for (std::map<std::string, std::string>::const_iterator it = _fields.begin(); it != _fields.end(); ++it)
		response << it->first << ": " << it->second << "\r\n";
	response << "\r\n";
	response << _content;

	_raw = response.str();

	return (0);
}

// Methods
void	HttpResponse::log(void) const
{
	log_message(Logger::DEBUG, "Response:");
	if (_contentType == "text/html" || _contentType == "text/css")
	{
		log_message(Logger::TRACE, "Response: %s", _raw.substr(0, 4096).c_str());
		if (_raw.size() > 4096)
			log_message(Logger::TRACE, "[...]");
	}
	else
		log_message(Logger::TRACE, "File \"%s\" not printable", _uri.c_str());
}

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

int	HttpResponse::setUp(HttpRequest const *request, const Config &config)
{
	_setRequest(request);
	if (_status != "200")
		return (_writeError(_status));
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
