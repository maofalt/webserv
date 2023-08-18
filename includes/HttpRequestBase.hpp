#ifndef HTTPREQUESTBASE_HPP
#define HTTPREQUESTBASE_HPP

#include <map>
// #include <string>
#include <time.h>
#include <vector>
#include <sstream>
#include <cstring>
#include <fstream>
#include <iostream>
#include <exception>
#include <algorithm>
#include <sys/types.h>
#include <sys/socket.h>


# define CRLF "\r\n"
# define CRLF_DOUBLE "\r\n\r\n"
# define ENDCHUNK "0\r\n\r\n"

# define CONTENT_LENGTH "Content-Length"
# define TRANSFER_ENCODING "Transfer-Encoding"

# define BUFFER_SIZE_REQUEST 4096
# define SIZE_MAX_REQUEST 32768

/*
Purpose:            To act as a base class for all HTTP request types.
Attributes/Methods: We already got a good starting point with the 
                    existing HttpRequestBase class. 
                    Refactor this to make it more generic 
                    (i.e., remove GET-specific behavior).
*/

class HttpRequestBase {
    public:
	// Method
		std::string	_method;
		std::string	_uri;
		std::string	_protocol;
	// Header
		std::map<std::string, std::string>	_field;
	// Body
		std::string	_body;
	// Utils
		std::string	_raw;
		bool		_headerComplete;
		bool		_bodyComplete;
	// Static
		static std::vector<std::string>				_methods_forbidden;
		static std::vector<std::string>				_methods_ok;
		static std::map<std::string, std::string>	_description;
		static std::map<std::string, std::string>	_content_type;

	// Methods utils
		void	_parseMethod(const std::string &method);
		void	_parseHeaderField(const std::string &field);
		void	_parseHeader(void);
		void	_parseBody(void);
		bool	_rawHeaderComplete(void) const;
		bool	_rawBodyComplete(void);
		void	_verifyHeader(void) const;

	public:
	// Coplien
		HttpRequestBase(void);
		HttpRequestBase(HttpRequestBase const &rhs);
		~HttpRequestBase(void);
		HttpRequestBase	&operator=(HttpRequestBase const &rhs);

	// Methods
		void	        clear(void);
		int		        recv(int fd);
		bool	        isComplete(void) const;
		virtual int		respond(int fd, std::string status);
        HttpRequestBase *createRequestObj(const std::string RequestType);

	// Operators
		friend std::ostream	&operator<<(std::ostream &out, const HttpRequestBase &rhs);

	// Exceptions
		class Error;
};

struct RequestsTab{
    std::string type;
    HttpRequestBase* (*createRequest)(void);
};

// Exceptions
class HttpRequestBase::Error : public std::exception
{
	private:
		char	_type[4];
	public:
		Error(const char type[4]) {memcpy(_type, type, 4);}
		const char	*what(void) const throw();
};

std::ostream& operator<<(std::ostream& os, const HttpRequestBase & other);
#endif