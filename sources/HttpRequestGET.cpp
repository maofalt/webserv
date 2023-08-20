/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequestGET.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: znogueir <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/20 14:41:22 by znogueir          #+#    #+#             */
/*   Updated: 2023/08/20 14:41:23 by znogueir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequestBase.hpp"
#include "HttpRequestGET.hpp"

HttpRequestGET::HttpRequestGET() : HttpRequestBase() {
}

HttpRequestGET::HttpRequestGET(int tmp_holder) : HttpRequestBase() {
    (void)tmp_holder;
}

HttpRequestGET::HttpRequestGET(const HttpRequestBase & Base) : HttpRequestBase(Base) {
}

HttpRequestGET::HttpRequestGET(const HttpRequestGET & other) : HttpRequestBase(other) {
    (void)other;
}

HttpRequestGET	&HttpRequestGET::operator=(const HttpRequestGET & other) {
    (void)other;
    return *this;
}

template <class T>
std::string	numberToString(T nb)
{
	std::ostringstream	ss;
	ss << nb;
	return (ss.str());
}

int	HttpRequestGET::respond(int fd, std::string status)
{
	std::string			response;
	std::stringstream	buffer;
	std::string			body;
	std::string			extension;

	std::cout << "\033[31mRequest:\033[0m" << std::endl;
	std::cout << *this << std::endl;

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
	body = buffer.str();

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
	response += numberToString(body.size());
	response += "\r\n";

	response += "Connection: ";
	response += "keep-alive";
	response += "\r\n";

	response += "Accept-Ranges: ";
	response += "bytes";
	response += "\r\n";

	response += "\r\n";

	response += body;

	std::cout << "\033[32mResponse:\033[0m" << std::endl;
	if (extension == "html" || extension == "css")
	{
		std::cout << response.substr(0, 4096) << std::endl;
		if (response.size() > 4096)
			std::cout << "[...]" << std::endl;
	}
	else
		std::cout << "File \"" << _uri << "\" not printable" << std::endl;

	send(fd, response.c_str(), response.size(), 0);
	return (0);
}
