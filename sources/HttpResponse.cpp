/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: znogueir <znogueir@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/24 21:55:01 by rgarrigo          #+#    #+#             */
/*   Updated: 2023/08/30 18:53:53 by znogueir         ###   ########.fr       */
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
	std::map<std::string, std::string>	content_type;

	content_type["html"] = "text/html";
	content_type["css"] = "text/css";
	content_type["jpg"] = "image/jpeg";
	content_type["jpeg"] = "image/jpeg";
	content_type["png"] = "image/png";
	return (content_type);
}
std::map<std::string, std::string>	HttpResponse::_description = getDescription();
std::map<std::string, std::string>	HttpResponse::_content_type = getContentType();

// Constructors
HttpResponse::HttpResponse(void):
	_protocol(DEFAULT_PROTOCOL),
	_i_raw(0)
{
}
HttpResponse::HttpResponse(uint16_t port):
	_port(port),
	_protocol(DEFAULT_PROTOCOL),
	_i_raw(0)
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
	_i_raw(rhs._i_raw)
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
	_i_raw = rhs._i_raw;

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

// Methods
int	HttpResponse::send(int fd)
{
	int	ret_value;

	if (_i_raw + SEND_BUFFER_SIZE <= _raw.size())
		return (::send(fd, _raw.c_str() + _i_raw, _raw.size() - _i_raw, 0));
	ret_value = ::send(fd, _raw.c_str() + _i_raw, SEND_BUFFER_SIZE, 0);
	_i_raw += SEND_BUFFER_SIZE;
	return (ret_value);
}

void	HttpResponse::setRequest(HttpRequest const *request)
{
	_request = request;
	_method = request->getMethod();
	_uri = request->getUri();
	_status = request->getStatus();
}

void	HttpResponse::setServer(const Config &config)
{
	_server = config.findServer(_request->getHost(), _port);
}

int	HttpResponse::respond(int fd, std::string status)
{
	std::string			response;
	std::stringstream	buffer;
	std::ostringstream	oss;
	std::string			extension;

	log_message(Logger::DEBUG, "Request:");
	oss << *_request << std::endl;
	log_message(Logger::TRACE, "Request: %s", oss.str().c_str());
	

	if (_uri == "/")
		_uri = "/index.html";

	// we try to open the requested page and if it fails we send a 404 error
	_uri = "./site2" + _uri;
	std::ifstream	file(_uri.c_str());
	if (!file.is_open())
	{
		ERROR_LOG("File not opened");
		_uri = "./site2/errors/404.html";
		status = "404";
		std::ifstream	file2(_uri.c_str());
		if (!file2.is_open())
			return (1);
		log_message(Logger::WARN, "404 opened");
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

// 3- Server: Information about the server CRLF
	response += "Server: ";
	response += "Webserv";
	response += "\r\n";

// 4- Content-Type: Type of the message body CRLF
	extension = _uri.substr(_uri.find_last_of(".") + 1);
	response += "Content-Type: ";
	response += _content_type[extension];
	response += "\r\n";

// 5- Content-Length: Size of the message body in bytes CRLF
	response += "Content-Length: ";
	response += numberToString(_content.size());
	response += "\r\n";

	response += "Connection: ";
	response += "keep-alive";
	response += "\r\n";

	response += "Accept-Ranges: ";
	response += "bytes";
	response += "\r\n";

	response += "\r\n";

	response += _content;

	log_message(Logger::DEBUG, "Response:");
	if (extension == "html" || extension == "css")
	{
		log_message(Logger::TRACE, "Response: %s", response.substr(0, 4096).c_str());
		if (response.size() > 4096)
			log_message(Logger::TRACE, "[...]");
	}
	else
		log_message(Logger::TRACE, "File \"%s\" not printable", _uri.c_str());

	::send(fd, response.c_str(), response.size(), 0);
	return (0);
}
