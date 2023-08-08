/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ayblin <ayblin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/03/10 06:18:27 by rgarrigo          #+#    #+#             */
/*   Updated: 2023/07/29 18:42:35 by rgarrigo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

# include <exception>
# include <iostream>
# include <map>
# include <string>
# include <vector>

# define CRLF "\r\n"
# define CRLF_DOUBLE "\r\n\r\n"
# define ENDCHUNK "0\r\n\r\n"

# define CONTENT_LENGTH "Content-Length"
# define TRANSFER_ENCODING "Transfer-Encoding"

# define BUFFER_SIZE_REQUEST 4096
# define SIZE_MAX_REQUEST 32768

//class Config
//{
//	public:
//		bool	oversized(const std::string &str) const {return (str.size() > 2048);}
//};

class HttpRequest
{
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
		HttpRequest(void);
		HttpRequest(HttpRequest const &rhs);
		~HttpRequest(void);
		HttpRequest	&operator=(HttpRequest const &rhs);

	// Methods
		void	clear(void);
		int		recv(int fd);
		bool	isComplete(void) const;
		int		respond(int fd, std::string status);

	// Operators
		friend std::ostream	&operator<<(std::ostream &out, const HttpRequest &rhs);

	// Exceptions
		class Error;
};

// Exceptions
class HttpRequest::Error : public std::exception
{
	private:
		char	_type[4];
	public:
		Error(const char type[4]) {memcpy(_type, type, 4);}
		const char	*what(void) const throw();
};

#endif
