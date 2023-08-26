/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: znogueir <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/20 14:42:13 by znogueir          #+#    #+#             */
/*   Updated: 2023/08/26 16:49:32 by rgarrigo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

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
                    existing HttpRequest class. 
                    Refactor this to make it more generic 
                    (i.e., remove GET-specific behavior).
*/

class HttpRequest {
    public:
	// Method
		std::string	_method;
		std::string	_uri;
		std::string	_protocol;
	// Header
		std::map<std::string, std::string>	_field;
	// Body
		std::string	_body;
	// Status
		std::string	_status;
	// Utils
		std::string	_raw;
		bool		_headerComplete;
		bool		_bodyComplete;
	// Static
		static std::vector<std::string>				_methods_forbidden;
		static std::vector<std::string>				_methods_ok;

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
		HttpRequest(void);
		HttpRequest(HttpRequest const &rhs);
		virtual ~HttpRequest(void);
		HttpRequest	&operator=(HttpRequest const &rhs);
	
	// Getters
		const std::string	getHost(void) const;

	// Methods
		void	        clear(void);
		int		        recv(int fd);
		bool	        isComplete(void) const;

	// Operators
		friend std::ostream	&operator<<(std::ostream &out, const HttpRequest &rhs);

	// Friends
		friend class HttpResponse;
};

std::ostream& operator<<(std::ostream& os, const HttpRequest & other);
#endif
