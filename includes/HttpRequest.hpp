/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/20 14:42:13 by znogueir          #+#    #+#             */
/*   Updated: 2023/09/07 18:22:56 by motero           ###   ########.fr       */
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
# include "Logger.hpp"
# include "Utils.hpp"



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
		int	_nextIt(std::string::iterator &it, const std::string::iterator &end);
		int	_parseMethod(const std::string &method);
		int	_parseHeaderField(const std::string &field);
		int	_parseHeader(void);
		int	_parseBody(void);
		bool	_rawHeaderComplete(void) const;
		bool	_rawBodyComplete(void);
		int	_setBuffer(std::string &buffer,
			std::string::iterator &it,
			const std::string::iterator &end);
		int	_verifyHeader(void);

	public:
	// Coplien
		HttpRequest(void);
		HttpRequest(HttpRequest const &rhs);
		virtual ~HttpRequest(void);
		HttpRequest	&operator=(HttpRequest const &rhs);
	
	// Getters
		const std::string	getHost(void) const;

	// Methods
		void	clear(void);
		void	log(void) const;
		int		recv(int fd);
		bool	isComplete(void) const;

	// Operators
		friend std::ostream	&operator<<(std::ostream &out, const HttpRequest &rhs);

	// Friends
		friend class HttpResponse;
};

std::ostream& operator<<(std::ostream& os, const HttpRequest & other);
#endif
