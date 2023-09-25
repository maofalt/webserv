/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/20 14:41:03 by znogueir          #+#    #+#             */
/*   Updated: 2023/09/25 01:26:59 by rgarrigo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequest.hpp"

// Static member variables
static std::vector<std::string>	getMethodsForbidden(void)
{
	std::vector<std::string>	methodsForbidden;

	methodsForbidden.push_back("HEAD");
	methodsForbidden.push_back("PUT");
	methodsForbidden.push_back("CONNECT");
	methodsForbidden.push_back("OPTIONS");
	methodsForbidden.push_back("TRACE");
	return (methodsForbidden);
}
static std::vector<std::string>	getMethodsOk(void)
{
	std::vector<std::string>	methodsOk;

	methodsOk.push_back("GET");
	methodsOk.push_back("POST");
	methodsOk.push_back("DELETE");
	return (methodsOk);
}

std::vector<std::string>			HttpRequest::_methods_forbidden = getMethodsForbidden();
std::vector<std::string>			HttpRequest::_methods_ok = getMethodsOk();

// Coplien
HttpRequest::HttpRequest(void):
	_method(""),
	_uri(""),
	_protocol(""),
	_field(),
	_body(""),
	_status("200"),
	_raw(""),
	_headerComplete(false),
	_bodyComplete(false)
{
}
HttpRequest::HttpRequest(HttpRequest const &rhs):
	_method(rhs._method),
	_uri(rhs._uri),
	_protocol(rhs._protocol),
	_field(rhs._field),
	_body(rhs._body),
	_status(rhs._status),
	_raw(rhs._raw),
	_headerComplete(rhs._headerComplete),
	_bodyComplete(rhs._bodyComplete)
{
}
HttpRequest::~HttpRequest(void)
{
}
HttpRequest	&HttpRequest::operator=(HttpRequest const &rhs)
{
	_method = rhs._method;
	_uri = rhs._uri;
	_protocol = rhs._protocol;
	_field = rhs._field;
	_body = rhs._body;
	_status = rhs._status;
	_raw = rhs._raw;
	_headerComplete = rhs._headerComplete;
	_bodyComplete = rhs._bodyComplete;
	return (*this);
}

// Utils
int	HttpRequest::_verifyHeader(void)
{
	volatile int	i = 0;

	(void)i;
	if (_field.count(CONTENT_LENGTH))
	{
		try
			{i = std::atoi(_field[CONTENT_LENGTH].c_str());}
		catch (std::exception &e)
			{return (_status = "400", -1);}
	}
	if (find(HttpRequest::_methods_forbidden.begin(), HttpRequest::_methods_forbidden.end(), _method) != HttpRequest::_methods_forbidden.end())
		return (_status = "405", -1);
	if (find(HttpRequest::_methods_ok.begin(), HttpRequest::_methods_ok.end(), _method) == HttpRequest::_methods_ok.end())
		return (_status = "501", -1);
	return (0);
}

int	HttpRequest::_setBuffer(std::string &buffer,
	std::string::iterator &it,
	const std::string::iterator &end)
{
	std::string				crlf(CRLF);
	std::string::iterator	it2;

	it2 = search(it, end, crlf.begin(), crlf.end());
	if (it2 == end)
		return (_status = "400", -1);
	buffer = std::string(it, it2);
	it = it2 + (crlf.end() - crlf.begin());
	return (0);
}

int	HttpRequest::_parseMethod(const std::string &method)
{
	size_t	first_sp;
	size_t	second_sp;

	first_sp = method.find_first_of(' ');
	if (first_sp == std::string::npos)
		return (_status = "400", -1);
	second_sp = method.find_first_of(' ', first_sp + 1);
	if (second_sp == std::string::npos)
		return (_status = "400", -1);
	_method = method.substr(0, first_sp);
	_uri = method.substr(first_sp + 1, second_sp - (first_sp + 1));
	_protocol = method.substr(second_sp + 1, std::string::npos);
	return (0);
}

int	HttpRequest::_parseHeaderField(const std::string &field)
{
	std::string	name;
	std::string	value;
	size_t		semicolon;
	size_t		first_char;
	size_t		last_char;

	semicolon = field.find_first_of(':');
	if (semicolon == std::string::npos)
		return (_status = "400", -1);
	name = field.substr(0, semicolon);
	value = field.substr(semicolon + 1, field.size());

	first_char = name.find_first_not_of(' ');
	last_char = name.find_last_not_of(' ');
	if (first_char == std::string::npos || last_char == std::string::npos)
		return (_status = "400", -1);
	name = name.substr(first_char, last_char + 1);

	first_char = value.find_first_not_of(' ');
	last_char = value.find_last_not_of(' ');
	if (first_char == std::string::npos || last_char == std::string::npos)
		return (_status = "400", -1);
	value = value.substr(first_char, last_char + 1);

	_field[name] = value;
	return (0);
}

int	HttpRequest::_parseHeader(void)
{
	std::string				buffer;
	std::string::iterator	it;

	it = _raw.begin();
	if (_setBuffer(buffer, it, _raw.end())
		|| _parseMethod(buffer) == -1
		|| _setBuffer(buffer, it, _raw.end()) == -1)
		return (-1);
	while (buffer.size() > 0)
	{
		if (_parseHeaderField(buffer) == -1
			|| _setBuffer(buffer, it, _raw.end()))
			return (-1);
	}
	if (_verifyHeader() == -1)
		return (-1);
	_raw.erase(_raw.begin(), it);
	return (0);
}

int	HttpRequest::_parseBody(void)
{
	if (_field.count(TRANSFER_ENCODING)
		&& _field[TRANSFER_ENCODING] == "chunked")
	{
		std::string::iterator	it = _raw.begin();
		std::string				endChunk(ENDCHUNK);
		std::string				crlf(CRLF);
		int						size;

		while (it != _raw.end())
		{
			if (it == search(it, _raw.end(), endChunk.begin(), endChunk.end()))
				return(0);
			size = std::atoi(&*it);
			it = search(it, _raw.end(), crlf.begin(), crlf.end());
			it += crlf.size();
			_body.append(it, it + size);
			it += size + crlf.size();
		}
	}
	else if (_field.count(CONTENT_LENGTH))
		_body = _raw.substr(0, std::atoi(_field[CONTENT_LENGTH].c_str()));
	return (0);
}

bool	HttpRequest::_rawHeaderComplete(void) const
{
	return (_raw.find(CRLF_DOUBLE) != std::string::npos);
}

int	HttpRequest::_nextIt(std::string::iterator &it, const std::string::iterator &end)
{
	std::string	crlf(CRLF);
	int	size;

	try
		{ size = std::atoi(&*it); }
	catch (std::exception &e)
		{ return (_status = "400", -1); }
	it = search(it, end, crlf.begin(), crlf.end());
	size += 2 * crlf.size();
	while (it != end && size--)
		++it;
	return (0);
}

bool	HttpRequest::_rawBodyComplete(void)
{
	if (_field.count(TRANSFER_ENCODING)
		&& _field[TRANSFER_ENCODING] == "chunked")
	{
		std::string::iterator	it = _raw.begin();
		std::string				endChunk(ENDCHUNK);

		while (it != _raw.end())
		{
			if (it == search(it, _raw.end(), endChunk.begin(), endChunk.end()))
				return (true);
			_nextIt(it, _raw.end());
		}
		return (false);
	}
	else if (_field.count(CONTENT_LENGTH))
		return ((int) _raw.size() >= std::atoi(_field[CONTENT_LENGTH].c_str()));
	return (true);
}

// Getters
const std::string	HttpRequest::getHost(void) const
{
	if (_field.count("Host"))
		return (_field.at("Host"));
	return ("");
}

// Methods
int	HttpRequest::recv(int fd)
{
	char	buffer[BUFFER_SIZE_REQUEST];
	int		count;

	count = ::recv(fd, buffer, BUFFER_SIZE_REQUEST, 0);
	if (count == 0)
		return (_status = "Closed", -1);
	if (count < 0)
		return (_status = "500", 0);
	_raw.append(buffer, count);
	if (!_headerComplete && _rawHeaderComplete())
	{
		if (_parseHeader() == -1)
			return (0);
		_headerComplete = true;
	}
	if (_headerComplete && _raw.size() > SIZE_MAX_REQUEST)
		return (_status = "413", 0);
	if (_headerComplete && !_bodyComplete && _rawBodyComplete())
	{
		_parseBody();
		_bodyComplete = true;
		return (0);
	}
	return (count);
}

bool	HttpRequest::isComplete(void) const
{
	return ((_headerComplete && _bodyComplete)
		|| _status != "200");
}

void	HttpRequest::clear(void)
{
	_method.clear();
	_uri.clear();
	_protocol.clear();
	_field.clear();
	_body.clear();
	_raw.clear();
	_headerComplete = false;
	_bodyComplete = false;
}

void	HttpRequest::log(void) const
{
	std::ostringstream	oss;

	oss << *this << std::endl;
	log_message(Logger::TRACE, "Request: %s", oss.str().c_str());
}

// Operators
std::ostream	&operator<<(std::ostream &out, const HttpRequest &rhs)
{
	out << "HTTP Request: " << std::endl;
	out << "Method: " << rhs._method << std::endl;
	out << "URI: " << rhs._uri << std::endl;
	out << "Protocol: " << rhs._protocol << std::endl;
	for (std::map<std::string, std::string>::const_iterator it = rhs._field.begin(); it != rhs._field.end(); ++it)
		out << it->first << ": " << it->second << std::endl;
	if (rhs._body.size() > 0)
	{
		out << "Body:" << std::endl;
		out << rhs._body;
	}
	else
		out << "Body: NONE" << std::endl;
	return (out);
}
