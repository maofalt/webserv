/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/24 21:55:01 by rgarrigo          #+#    #+#             */
/*   Updated: 2023/09/09 19:04:01 by rgarrigo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include "HttpResponse.hpp"

// Static member variables
#include <ctime>
static std::map<std::string, std::string>	getDescription(void)
{
	std::map<std::string, std::string>	description;

	std::srand(std::time(0));
	description["200"] = "OK";
	description["201"] = "Created";
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
	writeType[UPLOAD] = &HttpResponse::_writeUpload;
	writeType[DIRECTORY] = &HttpResponse::_writeDirectory;
	writeType[REDIRECTION] = &HttpResponse::_writeRedirection;
	writeType[AUTHENTIFICATION] = &HttpResponse::_writeAuthentification;
	writeType[ERROR] = &HttpResponse::_writeErrorBadRequest;
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
	_isSetUp(false);
	_iWriteToCgi(0),
	_pidCgi(0),
	_protocol(DEFAULT_PROTOCOL),
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
	_isSetUp(false);
	_iWriteToCgi(0),
	_pidCgi(0),
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
	_path(rhs._path),
	_uriIsDirectory(rhs._uriIsDirectory),
	_server(rhs._server),
	_location(rhs._location),
	_type(rhs._type),
	_isSetUp(rhs._isSetUp);
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
		kill(_pid);
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

// Getters
int	getFdCgiIn(void) const
{
	return (_fdCgiIn);
}
int	getFdCgiOut(void) const
{
	return (_fdCgiOut);
}
bool	isSetUp(void) const
{
	return (_isSetUp);
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
	if (_server->_maxSize && _request->_body.size() > _server->_maxSize)
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

// UploadFile
int	HttpResponse::_skipLine(std::string::size_type &i)
{
	i = _request->_body.find("\n", i);
	if (i == std::string::npos)
		return (-1);
	i++;
	return (0);
}
int	HttpResponse::_readUploadContentHeader(
	std::string &boundary,
	std::string &filename,
	std::string::size_type &i)
{
	std::string::size_type	j;
	bool					filenameSet;

	filenameSet = false;
	i = _request->_body.find(boundary, i);
	if (i == std::string::npos)
		return (-1);
	i += boundary.size();
	while (1)
	{
		if (_skipLine(i) == -1
			|| _request->_body.find(boundary, i) == i)
			return (-1);
		if (_request->_body.find("\r\n", i) == i
			||  _request->_body.find("\n", i) == i)
			break ;
		if (filenameSet
			|| _request->_body.find("Content-Disposition: form-data; ", i) != i
			|| _request->_body.find("filename=\"", i) > _request->_body.find("\n", i))
			continue ;
		i = _request->_body.find("filename=\"", i);
		if (i == std::string::npos)
			return (-1);
		i += 10;
		j = _request->_body.find("\"", i);
		if (j == std::string::npos)
			return (-1);
		filename = _request->_body.substr(i, j - i);
		filenameSet = true;
	}
	return (_skipLine(i));
}
int	HttpResponse::_readUploadContentBody(
	std::string &boundary,
	std::string &file,
	std::string::size_type &i)
{
	std::string::size_type	j;

	j = _request->_body.find(boundary, i);
	if (j >= i + 2
		&& _request->_body.find("\r\n", j - 2) == j - 2)
		j -= 2;
	else if (j >= i + 1
		&& _request->_body.find("\n", j - 1) == j - 1)
		j--;
	file = _request->_body.substr(i, j - i);
	i = j;
	return (0);
}
int	HttpResponse::_createFile(std::string &file, std::string &path)
{
	std::ofstream	fd;

	fd.open(path.c_str());
	if (!fd.is_open())
		return (ERROR_LOG("File not created"), -1);
	fd.write(file.c_str(), file.size());
	fd.flush();
	fd.close();
	return (0);
}
int	HttpResponse::_uploadFile(void)
{
	std::string				prefix("multipart/form-data; boundary=----");
	std::string				boundary("------");
	std::string				rootUpload;
	std::string				filename;
	std::string				path;
	std::string				file;
	std::string::size_type	i;

	if (_request->_field.at("Content-Type").find(prefix) != 0)
		return (2);
	boundary += _request->_field.at("Content-Type").substr(prefix.size(), std::string::npos);
	rootUpload = _location->_locConfig.at("upload")[1];
	if (*rootUpload.rbegin() != '/')
		rootUpload.push_back('/');
	i = 0;
	while (i != std::string::npos)
	{
		if (_readUploadContentHeader(boundary, filename, i) == -1)
			continue ;
		path = rootUpload + filename;
		if (access(path.c_str(), F_OK) != -1)
			continue ;
		if (_readUploadContentBody(boundary, file, i) == -1)
			continue ;
		_createFile(file, path);
	}
	return (0);
}

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
int	HttpResponse::_authentificate(void)
{
	std::istringstream			ss(_request->_body);
	std::string					name;
	std::string					value;
	std::string					role;
	std::string					user;
	std::string					password;
	std::vector<std::string>	rolesRequested;
	std::vector<std::string>	rolesAccepted;

	if (!_request->_field.count("Content-Type")
		|| _request->_field.at("Content-Type") != "application/x-www-form-urlencoded")
		return (2);
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
	if (user == ""
		|| password == ""
		|| rolesRequested.size() == 0)
		return (2);
	_server->_credentials["brogarow"]["admin"] = "password";
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
	_cookie = _generateCookie();
	_server->_sessionCookie[_cookie]["user"] = user;
	for (std::vector<std::string>::const_iterator role = rolesAccepted.begin(); role != rolesAccepted.end(); ++role)
		_server->_sessionCookie[_cookie][*role] = "on";
	return (0);
}

int	HttpResponse::_setEnvCgi(void)
{
	std::ostringstream	variable("");

	variable << "SERVER_SOFTWARE=" << SERVER_NAME << "/" << SERVER_VERSION;
	_envCgi.push_back(variable.str());
	variable.str("");

	variable << "SERVER_NAME=" << _host;
	_envCgi.push_back(variable.str());
	variable.str("");

	variable << "GATEWAY_INTERFACE=CGI/1.1";
	_envCgi.push_back(variable.str());
	variable.str("");

	variable << "REQUEST_METHOD=" << _method;
	_envCgi.push_back(variable.str());
	variable.str("");

	variable << "SCRIPT_NAME=" << _uri;
	_envCgi.push_back(variable.str());
	variable.str("");

	variable << "SCRIPT_FILENAME=" << _path;
	_envCgi.push_back(variable.str());
	variable.str("");

	variable << "REQUEST_FILENAME=" << _path;
	_envCgi.push_back(variable.str());
	variable.str("");

	variable << "QUERY_STRING=" << _queryString;
	_envCgi.push_back(variable.str());
	variable.str("");

	variable << "REDIRECT_STATUS=";
	_envCgi.push_back(variable.str());
	variable.str("");

	if (_method == "POST")
	{
		if (_request->_field.count("Content-Type"))
		{
			variable << "CONTENT_TYPE=" << _request->_field.at("Content-Type");
			_envCgi.push_back(variable.str());
			variable.str("");
		}

		variable << "CONTENT_LENGTH=" << _request->_body.size();
		_envCgi.push_back(variable.str());
		variable.str("");
	}

	return (0);
}
int	HttpResponse::_launchCgi(void)
{
	int		pid;
	int		pipeFdIn[2];
	int		pipeFdOut[2];

	if (_uploadFileOn)
		_uploadFile();
	if (pipe(pipeFdIn) == -1)
		return (_writeError("500"));
	if (pipe(pipeFdOut) == -1)
		return (close(pipeFdIn[0]), close(pipeFdIn[1]), _writeError("500"));
	pid = fork();
	if (pid == -1)
		return (close(pipeFdIn[0]), close(pipeFdIn[1]), close(pipeFdOut[0]), close(pipeFdOut[1]), _writeError("500"));
	if (pid == 0)
	{
		close(pipeFdIn[1]);
		close(pipeFdOut[0]);
		if (dup2(pipeFdIn[0], 0) == -1)
			return (close(pipeFdIn[0]), close(pipeFdOut[1]), -1);
		close(pipeFdIn[0]);
		if (dup2(pipeFdOut[1], 1) == -1)
			return (close(pipeFdOut[1]), -1);
		close(pipeFdOut[1]);
		std::string	pathExec;
		char		*argv[3];
		char		*env[100];

		pathExec = _location->_locConfig.at("cgi")[1];
		argv[0] = &pathExec[0];
		if (_path.find("./", 0) == 0)
		{
			_path.erase(0, 2);
			_path.insert(0, "/");
			_path.insert(0, std::getenv("PWD"));
		}
		argv[1] = &_path[0];
		argv[2] = NULL;
		_setEnvCgi();
		int	i = 0;
		for (std::vector<std::string>::iterator it = _envCgi.begin(); i < 99 && it != _envCgi.end(); ++it)
			env[i++] = &((*it)[0]);
		env[i] = NULL;
		chdir(_path.substr(0, _path.find_last_of("/")).c_str());
		execve(argv[0], argv, env);
	}
	_pidCgi = pid;
	close(pipeFdIn[0]);
	close(pipeFdOut[1]);
	_fdCgiIn = pipeFdIn[1];
	_fdCgiOut = pipeFdOut[0];
	return (CGI_LAUNCHED);
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
	_fields["ContentType"] = "text/html";
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
	_fields["ContentType"] = "text/html";
	return (_writeRaw());
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
	_fields["ContentType"] = _mapContentType[extension];

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
	_fields["ContentType"] = "text/html";
	return (_writeRaw());
}
int	HttpResponse::_writeErrorBadRequest(void)
{
	return (_writeError("400"));
}
int	HttpResponse::_writeError(std::string status)
{
	_status = status;
	_content = _defaultErrorPages[status];
	_fields["ContentType"] = "text/html";
	return (_writeRaw());
}
int	HttpResponse::_writeUpload(void)
{
	std::ostringstream	content;

	if (_uploadFile())
		return (_writeError("400"));
	_status = "201";
	content << "<!DOCTYPE html>\n"
		<< "<html lang=\"en\">\n"
		<< "<head>\n"
		<< "	<meta charset=\"utf-8\" />\n"
		<< "	<title>Uploaded</title>\n"
		<< "</head>\n"
		<< "<body>\n"
		<< "	<p>File has been uploaded.</p>\n"
		<< "</body>\n"
		<< "</html>\n";
	_content = content.str();
	_fields["ContentType"] = "text/html";
	return (_writeRaw());
}
int	HttpResponse::_writeAuthentification(void)
{
	std::ostringstream	content;
	int					status;

	status = _authentificate();
	if (status == 2)
		return (_writeError("400"));
	if (status == 3)
		return (_writeError("401"));
	_status = "201";
	_fields["Set-Cookie"] = "authentification=";
	_fields["Set-Cookie"] += _cookie;
	content << "<!DOCTYPE html>\n"
		<< "<html lang=\"en\">\n"
		<< "<head>\n"
		<< "	<meta charset=\"utf-8\" />\n"
		<< "	<title>Authentification success</title>\n"
		<< "</head>\n"
		<< "<body>\n"
		<< "	<p>Authentification success.</p>\n"
		<< "</body>\n"
		<< "</html>\n";
	_content = content.str();
	_fields["ContentType"] = "text/html";
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
	response << "Content-Length: " << numberToString(_content.size()) << "\r\n";
	response << "Connection: " << "keep-alive" << "\r\n";
	response << "Accept-Ranges: " << "bytes" << "\r\n";
	for (std::map<std::string, std::string>::const_iterator it = _fields.begin(); it != _fields.end(); ++it)
		response << it->first << ": " << it->second << "\r\n";
	if (_type != CGI)
		response << "\r\n";
	response << _content;

	_raw = response.str();

	return (0);
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
		return (close(_fdCgiIn), 0);
	return (1);
}
int	HttpResponse::readCgi(bool timeout)
{
	char	buffer[READ_BUFFER_SIZE] = {};
	int		bytesRead;

	if (timeout)
		return (close(_fdCgiIn), close(_fdCgiOut), _writeError("504"));
	bytesRead = read(_fdCgiOut, buffer, READ_BUFFER_SIZE);
	if (bytesRead == -1)
		return (close(_fdCgiOut), waitpid(_pidCgi, NULL, 0), _writeError("500"));
	_content.append(buffer, bytesRead);
	if (bytesRead < READ_BUFFER_SIZE)
		return (close(_fdCgiOut), waitpid(_pidCgi, NULL, 0),  _writeRaw());
	return (bytesRead);
}

int	HttpResponse::setUp(HttpRequest const *request, Config &config)
{
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
