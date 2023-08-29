/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/20 14:41:03 by znogueir          #+#    #+#             */
/*   Updated: 2023/08/29 17:20:17 by motero           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequest.hpp"

// #include "HttpRequest.hpp"

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

// _verifyHeader
void	HttpRequest::_verifyHeader(void) const
{
	if (find(HttpRequest::_methods_forbidden.begin(), HttpRequest::_methods_forbidden.end(), _method) != HttpRequest::_methods_forbidden.end())
		throw (Error("405"));
	if (find(HttpRequest::_methods_ok.begin(), HttpRequest::_methods_ok.end(), _method) == HttpRequest::_methods_ok.end())
		throw (Error("501"));
}

/* 
	_parseHeader
	searches for the next CRLF in the buffer,
	and sets the value of the buffer to the string between the current iterator
	and the CRLF('\r\n'), CRLF indicates the end of the HEADER.
	 If the Transfer-Encoding: chunked header is used, the CRLF sequence is also
	 used to separate chunks of the message body.
	 The iterator is then moved to the end of the CRLF

	 ?? ; do we account for chunked transmission or we suppose that the header 
	 is contained in only one request?
*/
static void	setBuffer(std::string &buffer,
	std::string::iterator &it,
	const std::string::iterator &end)
{
	std::string				crlf(CRLF);
	std::string::iterator	it2;

	it2 = search(it, end, crlf.begin(), crlf.end());
	if (it2 == end)
		throw (HttpRequest::Error("400"));
	buffer = std::string(it, it2);
	it = it2 + (crlf.end() - crlf.begin());
}

/*
	We extract the request line here, which includes the HTTP method 
	(e.g., GET, POST), the request URI, and the HTTP version
	?? : Should we verify that the htpp version is 1.1?
*/
void	HttpRequest::_parseMethod(const std::string &method)
{
	size_t	first_sp;
	size_t	second_sp;

	//We extract the  the method between the spaces
	first_sp = method.find_first_of(' ');
	if (first_sp == std::string::npos)
		throw (Error("400"));
	second_sp = method.find_first_of(' ', first_sp + 1);
	if (second_sp == std::string::npos)
		throw (Error("400"));
	_method = method.substr(0, first_sp);
	//Since we already subtracted the CRLF, the middle part is the uri
	_uri = method.substr(first_sp + 1, second_sp - (first_sp + 1));
	// last part is the protocol (HTTP/1.1 )
	_protocol = method.substr(second_sp + 1, std::string::npos);
}

/* Pase of the a header field spliting name and value and saving it in the _field map
	it is colon, semicolon is used to separet multiple values
	?? :  if the value is empty, should we throw an error? I supppose this is done in the verification part
	?? : is  the value ahs different values  separeted by a coma,
	should we split it and save it in a vector? Or should we instead  use a multimap ?
	?? : if there are semicolon (diffrent parameters) should we split them or 
	keep them as a unique value ?
*/
void	HttpRequest::_parseHeaderField(const std::string &field)
{
	// Find the first semicolon to separate the name and value
	std::string	name;
	std::string	value;
	size_t		semicolon;
	size_t		first_char;
	size_t		last_char;

	semicolon = field.find_first_of(':');
	if (semicolon == std::string::npos)
		throw (Error("400"));
	name = field.substr(0, semicolon);
	value = field.substr(semicolon + 1, field.size());

	// Remove leading and trailing whitespaces from the name and value
	first_char = name.find_first_not_of(' ');
	last_char = name.find_last_not_of(' ');
	if (first_char == std::string::npos || last_char == std::string::npos)
		throw (Error("400"));
	name = name.substr(first_char, last_char + 1);

	first_char = value.find_first_not_of(' ');
	last_char = value.find_last_not_of(' ');
	if (first_char == std::string::npos || last_char == std::string::npos)
		throw (Error("400"));
	value = value.substr(first_char, last_char + 1);

	// Add the name and value to the _field map
	_field[name] = value;
}

/*
We extract the  header, triming the CRLF at the end of the header
Then we parse the method, uri and protocol
Then we parse the header fields
?? : do we need to verify anything else besides if the methods is correct ? 
*/
void	HttpRequest::_parseHeader(void)
{
	std::string				buffer;
	std::string::iterator	it;

	it = _raw.begin();
	setBuffer(buffer, it, _raw.end());
	_parseMethod(buffer);
	//buffer to next header field
	setBuffer(buffer, it, _raw.end());
	while (buffer.size() > 0)
	{
		_parseHeaderField(buffer);
		setBuffer(buffer, it, _raw.end());
	}
	//We verify if the method is allowed
	_verifyHeader();
	_raw.erase(_raw.begin(), it);
}

// _parseBody
/*
	?? : We may need to handle other types of transfer encoding as errors,
rather than implementing them
	?? : we should handle if the content length is incorrect.  
	error ? we ignore the bdy?send a timeout suing 413 error ?
	
*/
void	HttpRequest::_parseBody(void)
{
	// When the body is chunked, the size of the chunked data is unknown
	if (_field.count(TRANSFER_ENCODING)
		&& _field[TRANSFER_ENCODING] == "chunked")
	{
		std::string::iterator	it = _raw.begin();
		std::string				endChunk(ENDCHUNK);
		std::string				crlf(CRLF);
		int						size;

		while (it != _raw.end())
		{
			std::cout << "in body while" << std::endl;
			// If the end chunk is found, the parsing is complete.
			if (it == search(it, _raw.end(), endChunk.begin(), endChunk.end()))
				return ;
			// Get the size of the chunked data.
			size = std::atoi(&*it);
			// Get the start position of the chunked data
			it = search(it, _raw.end(), crlf.begin(), crlf.end());
			// Skip the size and CRLF.
			it += crlf.size();
			// Append chunked data to the body.
			_body.append(it, it + size);
			// Skip the chunked data and CRLF.
			it += size + crlf.size();
		}
	}
	// When the body is not chunked
	else if (_field.count(CONTENT_LENGTH))
		_body = _raw.substr(0, std::atoi(_field[CONTENT_LENGTH].c_str()));
}

// _rawHeaderComplete
bool	HttpRequest::_rawHeaderComplete(void) const
{
	return (_raw.find(CRLF_DOUBLE) != std::string::npos);
}

// _rawBodyComplete
static void	nextIt(std::string::iterator &it, const std::string::iterator &end)
{
	std::string	crlf(CRLF);
	int	size;

	// 1. Get the size of the next field
	try { size = std::atoi(&*it); }
	catch (std::exception &e) {
		throw (HttpRequest::Error("400")); }
	// 2. Find the end of the field
	it = search(it, end, crlf.begin(), crlf.end());
	// 3. Skip the field
	size += 2 * crlf.size();
	while (it != end && size--)
		++it;
}


bool	HttpRequest::_rawBodyComplete(void)
{
	// If Transfer-Encoding is chunked, search for a double CRLF
	if (_field.count(TRANSFER_ENCODING)
		&& _field[TRANSFER_ENCODING] == "chunked")
	{
		std::string::iterator	it = _raw.begin();
		std::string				endChunk(ENDCHUNK);

		// Search the string for the end of the last chunk
		while (it != _raw.end())
		{
			if (it == search(it, _raw.end(), endChunk.begin(), endChunk.end()))
				return (true);
			nextIt(it, _raw.end());
		}
		return (false);
	}
	// If Content-Length is set, compare the size of the raw body to the value
	// instead of  >= couldn't we use =! ? client is allowed to send somethign smaller ?
	else if (_field.count(CONTENT_LENGTH))
	{
		try {
			return ((int) _raw.size() >= std::atoi(_field[CONTENT_LENGTH].c_str()));
		} catch (std::exception &e) {
			throw (Error("400")); }
	}
	// If no headers are set, the body is complete
	return (true);
}

// Getters
const std::string	HttpRequest::getHost(void) const
{
	if (_field.count("Host"))
		return (_field.at("Host"));
	return ("");
}
const std::string	&HttpRequest::getMethod(void) const
{
	return (_method);
}
const std::string	&HttpRequest::getStatus(void) const
{
	return (_status);
}
const std::string	&HttpRequest::getUri(void) const
{
	return (_uri);
}

// Methods

int	HttpRequest::recv(int fd)
{
	char	buffer[BUFFER_SIZE_REQUEST + 1];
	int		count;

	std::memset(buffer, 0, BUFFER_SIZE_REQUEST + 1);
	log_message(Logger::DEBUG, "before recv");
	count = ::recv(fd, buffer, BUFFER_SIZE_REQUEST, 0);
	log_message(Logger::DEBUG_DETAILED, "after recv");
	if (count == 0)
		throw (Error("LOL"));
	if (count < 0)
		throw (Error("500"));
	_raw += buffer;
	// all headers are parsed, trimmed from the raw request and saved in the _field map
	if (!_headerComplete && _rawHeaderComplete())
	{
		// std::cout << "parse Header" << std::endl;
		_parseHeader();
		_headerComplete = true;
	}
	// Request entity too large
	if (_headerComplete && _raw.size() > SIZE_MAX_REQUEST)
		throw (Error("413"));
	//as long as the body is not complete, we parse it
	if (_headerComplete && !_bodyComplete && _rawBodyComplete())
	{
		_parseBody();
		_bodyComplete = true;
	}
	return (count);
}

// isComplete
bool	HttpRequest::isComplete(void) const
{
	return (_headerComplete && _bodyComplete);
}

// clear
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

// operator<<
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

// Exception
const char	*HttpRequest::Error::what(void) const throw()
{
	return (_type);
}
