/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse_write.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgarrigo <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/25 01:28:16 by rgarrigo          #+#    #+#             */
/*   Updated: 2023/10/10 15:29:04 by rgarrigo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpResponse.hpp"

// Static member variables
static std::map<std::string, std::string>	getDescription(void)
{
	std::map<std::string, std::string>	description;

	description["200"] = "OK";
	description["201"] = "Created";
	description["204"] = "No Content";
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
		<< "	<link rel=\"icon\" type=\"image/png\" href=\"/motero.png\"/>\n"
		<< "</head>\n"
		<< "<body>\n"
		<< "	<p>This page has moved to <a href=\"" << _fields["Location"]
		<< "\">" << _fields["Location"] << "</a>.</p>\n"
		<< "</body>\n"
		<< "</html>\n";
	_content = content.str();
	_fields["Content-Type"] = "text/html";
	return (_writeRaw());
}

int	HttpResponse::_writeDirectory(void)
{
	std::ostringstream	content;
	DIR					*dir;
	struct dirent		*entry;

	content << "<!DOCTYPE html>\n"
			<< "<html>\n"
			<< "	\n"
			<< "<head>\n"
			<< "	<meta charset=\"utf-8\" />\n"
			<< "	<title>CGI testing</title>\n"
			<< "	<link rel=\"stylesheet\" href=\"/css/style_cgi_testing.css\"/>\n"
			<< "	<link rel=\"icon\" type=\"image/png\" href=\"/motero.png\"/>\n"
			<< "</head>\n"
			<< "\n"
			<< "<body>\n"
			<< "	<header>\n"
			<< "		<h1>Webserv: ." + _uri + "</h1>\n"
			<< "		<div>\n";
	dir = opendir(_path.c_str());
	entry = readdir(dir);
	while (entry)
	{
		if (entry->d_name[0] == '.')
		{
			entry = readdir(dir);
			continue ;
		}
		content << "			<h2>\n"
				<< "				<a href=\"" << _uri << entry->d_name << "\"><span class=\"h2\"</span>" << entry->d_name << "</a>\n"
				<< "			</h2>\n";
		entry = readdir(dir);
	}
	closedir(dir);
	content	<< "			<h2 id=\"back\">\n"
			<< "				<a href=\"../index.html\"  title=\"Back to index\"><span class=\"h2\">< back&nbsp;</span></a>\n"
			<< "			</h2>\n"
			<< "		</div>\n"
			<< "	</header>\n"
			<< "\n"
			<< "	<footer>\n"
			<< "		<p>Rgarrigo&nbsp;&nbsp;&nbsp;Motero&nbsp;&nbsp;&nbsp;Znogueir</p>\n"
			<< "	</footer>\n"
			<< "</body>\n"
			<< "\n"
			<< "</html>\n";
	_content = content.str();
	_fields["Content-Type"] = "text/html";
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
	_fields["Content-Type"] = _mapContentType[extension];

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
		<< "	<link rel=\"icon\" type=\"image/png\" href=\"/motero.png\"/>\n"
		<< "</head>\n"
		<< "<body>\n"
		<< "	<p>File has been deleted.</p>\n"
		<< "</body>\n"
		<< "</html>\n";
	_content = content.str();
	_fields["Content-Type"] = "text/html";
	return (_writeRaw());
}

int	HttpResponse::_writeErrorBadRequest(void)
{
	return (_writeError("400"));
}

int	HttpResponse::_writeError(std::string status)
{
	_status = status;
	if (_server && _server->_servConfig.count(status))
	{
		std::ifstream	errPage(_server->_servConfig.at(status)[1].c_str());
		if (errPage.is_open())
		{
			std::stringstream	buffer;

			buffer << errPage.rdbuf();
			errPage.close();
			_content = buffer.str();
		}
		else
			_content = _defaultErrorPages[status];
	}
	else
		_content = _defaultErrorPages[status];
	_fields["Content-Type"] = "text/html";
	if (_status == "408")
		_fields["Connection"] = "close";
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
		<< "	<link rel=\"icon\" type=\"image/png\" href=\"/motero.png\"/>\n"
		<< "</head>\n"
		<< "<body>\n"
		<< "	<p>File has been uploaded.</p>\n"
		<< "</body>\n"
		<< "</html>\n";
	_content = content.str();
	_fields["Content-Type"] = "text/html";
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
		<< "	<link rel=\"icon\" type=\"image/png\" href=\"/motero.png\"/>\n"
		<< "</head>\n"
		<< "<body>\n"
		<< "	<p>Authentification success.</p>\n"
		<< "</body>\n"
		<< "</html>\n";
	_content = content.str();
	_fields["Content-Type"] = "text/html";
	return (_writeRaw());
}

int	HttpResponse::_writeCgi(void)
{
	std::istringstream	ss(_outputCgi);
	std::string			field;

	std::getline(ss, field, '\n');
	while (field != "" && field != "\r")
	{
		if (field == "204" || field == "204\r")
		{
			_status = "204";
			std::getline(ss, field, '\n');
			continue ;
		}
		std::istringstream	ssLine(field);
		std::string			name;
		std::string			value;

		std::getline(ssLine, name, ':');
		std::getline(ssLine, value, '\0');
		if (name == "Set-Cookie" && _fields.count("Set-Cookie") == 1)
		{
			_fields["Set-Cookie"] += "\r\n";
			_fields["Set-Cookie"] += "Set-Cookie: ";
			_fields["Set-Cookie"] += value;
		}
		else
			_fields[name] = value;
		std::getline(ss, field, '\n');
	}
	std::getline(ss, _content, '\0');
	return (_writeRaw());
}

template <class T>
std::string	numberToString(T nb)
{
	std::ostringstream	ss;
	ss << nb;
	return (ss.str());
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
	if (_content.size() > 0)
		response << "Content-Length: " << numberToString(_content.size()) << "\r\n";
	if (_fields.count("Connection") == 0)
		response << "Connection: " << "keep-alive" << "\r\n";
	response << "Accept-Ranges: " << "bytes" << "\r\n";
	for (std::map<std::string, std::string>::const_iterator it = _fields.begin(); it != _fields.end(); ++it)
		response << it->first << ": " << it->second << "\r\n";
	response << "\r\n";
	response << _content;

	_raw = response.str();

	return (0);
}
