/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/24 21:48:54 by rgarrigo          #+#    #+#             */
/*   Updated: 2023/09/20 18:03:07 by rgarrigo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPRESPOND_HPP
# define HTTPRESPOND_HPP

# include <cstdio>
# include <cstdlib>
# include <dirent.h>
# include <map>
# include <signal.h>
# include <stdint.h>
# include <string>
# include <sys/types.h>
# include <sys/wait.h>
# include <unistd.h>
# include "Config.hpp"
# include "HttpRequest.hpp"
# include "Logger.hpp"

# define DEFAULT_PROTOCOL "HTTP/1.1"

# define READ_BUFFER_SIZE 16384
# define SEND_BUFFER_SIZE 16384
# define WRITE_CGI_BUFFER_SIZE 2048

# define RESPONSE_SET 0
# define CGI_LAUNCHED 1

# define SERVER_NAME "webserv"
# define SERVER_VERSION "1.0"

typedef	enum e_response_type
{
	GET,
	DELETE,
	UPLOAD,
	DIRECTORY,
	REDIRECTION,
	CGI,
	AUTHENTIFICATION,
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
		std::string							_path;
		bool								_uriIsDirectory;
		std::string							_queryString;
		ServerConfig						*_server;
		const t_location					*_location;

	// Internal
		std::set<int>						_fdsToClose;
		t_responseType						_type;
		bool								_isSetUp;
		int									_fdCgiIn;
		std::string::size_type				_iWriteToCgi;
		std::string							_outputCgi;
		int									_fdCgiOut;
		int									_pidCgi;
		std::vector<std::string>			_envCgi;
		bool								_uploadFileOn;
		bool								_uploadOnly;
		std::string							_cookie;

	// Content
		std::string							_protocol;
		std::string							_status;
		std::map<std::string, std::string>	_fields;
		std::string							_content;
		std::string							_raw;
		std::string::size_type				_iRaw;

	// Static
		static std::map<std::string, std::string>							_description;
		static std::map<std::string, std::string>							_mapContentType;
		static std::map<t_responseType, t_writeType>						_writeType;
		static std::map<std::string, std::string>							_defaultErrorPages;

	// UploadFile
		int	_skipLine(std::string::size_type &i);
		int	_readUploadContentHeader(
			std::string &boundary,
			std::string &filename,
			std::string::size_type &i);
		int	_readUploadContentBody(
			std::string &boundary,
			std::string &file,
			std::string::size_type &i);
		int	_createFile(std::string &file, std::string &path);
		int	_uploadFile(void);

	// Utils
		int			_authentificate(void);
		int			_checkPath(void);
		int			_determineLocation(void);
		int			_determinePost(void);
		std::string	_generateCookie(void);
		int			_launchSon(int pipeFdIn[2], int pipeFdOut[2]);
		int			_launchCgi(void);
		int			_limitClientBodySize(void);
		int			_limitHttpMethod(void);
		bool		_locationAllowed(void);
		int			_refineUri(void);
		int			_setEnvCgi(void);
		int			_setRequest(const HttpRequest *request);
		int			_setServer(Config &config);
		int			_setType(void);
		int			_stripUri(void);
		int			_writeAuthentification(void);
		int			_writeCgi(void);
		int			_writeDelete(void);
		int			_writeDirectory(void);
		int			_writeErrorBadRequest(void);
		int			_writeGet(void);
		int			_writeRedirection(void);
		int			_writeUpload(void);
		int			_writeError(std::string status);
		int			_writeRaw(void);

	// Utils types
		static std::map<t_responseType, t_writeType>	_getWriteType(void);

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
		int		getFdCgiIn(void) const;
		int		getFdCgiOut(void) const;
		bool	isSetUp(void) const;

	// Methods
		void	log(void) const;
		int		readCgi(bool timeout);
		int		writeToCgi(void);
		int		send(int fd);
		int		setUp(HttpRequest const *request, Config &config);

	// Static
		static std::map<std::string, std::string>	getDefaultErrorPages(void);
};

#endif
