/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgarrigo <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/24 21:48:54 by rgarrigo          #+#    #+#             */
/*   Updated: 2023/08/26 14:21:14 by rgarrigo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPRESPOND_HPP
# define HTTPRESPOND_HPP

# include <stdint.h>
# include <string>
# include "Config.hpp"
# include "HttpRequest.hpp"
# include "Logger.hpp"

# define DEFAULT_PROTOCOL "HTTP/1.1"

# define SEND_BUFFER_SIZE 16384

typedef	enum e_response_type
{
	REDIRECTION,
	DIRECTORY,
	CGI,
	GET,
	DELETE,
	ERROR
}	t_response_type;

class HttpResponse
{
	private:
	// Arguments
		uint16_t							_port;
		const HttpRequest					*_request;
		std::string							_method;
		std::string							_uri;
		std::map<std::string, std::string>	_parameters;
		const ServerConfig					*_server;

	// Temp
		t_response_type						_type;
		int									_fdCgi;

	// Content
		std::string							_protocol;
		std::string							_status;
		std::map<std::string, std::string>	_fields;
		std::string							_content;
		std::string							_raw;
		std::string::size_type				_i_raw;

	// Static
		static std::map<std::string, std::string>	_description;
		static std::map<std::string, std::string>	_content_type;

	// Utils
		int	_limitClientBodySize(void);
		int	_limitHttpMethod(void);
		int	_refineUri(void);
		int	_setType(void) const;
		int	_writeRedirection(void);
		int	_writeDirectory(void);
		int	_writeCgi(void);
		int	_writeGet(void);
		int	_writeDelete(void);

	public:
	// Constructors
		HttpResponse(void);
		HttpResponse(uint16_t port);
		HttpResponse(HttpResponse const &rhs);
		HttpResponse(HttpRequest const *request);

	// Destructor
		~HttpResponse(void);

	// Operators
		HttpResponse	&operator=(HttpResponse const &rhs);

	// Getters
		int		getFdCgi(void) const;

	// Methods
		int		send(int fd);
		void	setRequest(HttpRequest const *request);
		void	setServer(const Config &config);
		int		write(void);
		int		writeAfterCgi(bool timeout);

	// Temp
		int	respond(int fd, std::string status);
};

#endif
