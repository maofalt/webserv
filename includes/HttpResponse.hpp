/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgarrigo <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/24 21:48:54 by rgarrigo          #+#    #+#             */
/*   Updated: 2023/08/31 20:11:30 by rgarrigo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPRESPOND_HPP
# define HTTPRESPOND_HPP

# include <map>
# include <stdint.h>
# include <string>
# include <unistd.h>
# include "Config.hpp"
# include "HttpRequest.hpp"
# include "Logger.hpp"

# define DEFAULT_PROTOCOL "HTTP/1.1"

# define READ_BUFFER_SIZE 16384
# define SEND_BUFFER_SIZE 16384

typedef	enum e_response_type
{
	GET,
	DELETE,
	DIRECTORY,
	REDIRECTION,
	CGI,
	ERROR
}	t_responseType;

class HttpResponse;
typedef int (HttpResponse::*t_writeType)(void);

class HttpResponse
{
	private:
	// Arguments
		uint16_t							_port;
		const HttpRequest					*_request;
		std::string							_method;
		std::string							_host;
		std::string							_uri;
		bool								_uriIsDirectory;
		std::map<std::string, std::string>	_parameters;
		const ServerConfig					*_server;
		const t_location					*_location;

	// Temp
		t_responseType						_type;
		int									_fdCgi;

	// Content
		std::string							_protocol;
		std::string							_status;
		std::map<std::string, std::string>	_fields;
		std::string							_content;
		std::string							_raw;
		std::string::size_type				_iRaw;

	// Static
		static std::map<std::string, std::string>	_description;
		static std::map<std::string, std::string>	_contentType;

	// Utils
		int	_determineLocation(void);
		int	_launchCgi(void);
		int	_limitClientBodySize(void);
		int	_limitHttpMethod(void);
		int	_refineUri(void);
		int	_setRequest(const HttpRequest *request);
		int	_setServer(const Config &config);
		int	_setType(void);
		int	_stripUri(void);
		int	_writeRedirection(void);
		int	_writeDirectory(void);
		int	_writeCgi(void);
		int	_writeGet(void);
		int	_writeDelete(void);
		int	_writeType(void);
		int	_writeError(std::string status);
		std::map<t_responseType, t_writeType>	_getWriteType(void);

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
		int		readCgi(bool timeout);
		int		send(int fd);
		int		setUp(HttpRequest const *request, const Config &config);

	// Temp
		int	respond(int fd, std::string status);
};

#endif
