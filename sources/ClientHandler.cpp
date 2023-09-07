/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientHandler.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/24 23:16:17 by rgarrigo          #+#    #+#             */
/*   Updated: 2023/09/07 23:39:12 by rgarrigo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ClientHandler.hpp"

Config					ClientHandler::_config;
std::map<int, uint16_t>	ClientHandler::_port;

// Coplien
ClientHandler::~ClientHandler() 
{
}
ClientHandler::ClientHandler(void)
{
}
ClientHandler::ClientHandler(const ClientHandler& other) : 
	_client_fd(other._client_fd),
	_request(other._request),
	_response(other._response)
{
}
ClientHandler&	ClientHandler::operator=(const ClientHandler& other)
{
	if (this != &other) {
		_client_fd = other._client_fd;
		_request = other._request;
		_response = other._response;
	}
	return *this;
}

// Constructor
ClientHandler::ClientHandler(int fdSock, int fd) :
	_client_fd(fd),
	_request(),
	_response(_port[fdSock])
{
}

// Static
void	ClientHandler::addPort(int fdSock, uint16_t port)
{
	ClientHandler::_port[fdSock] = port;
}
void	ClientHandler::setConfig(const Config &config)
{
	ClientHandler::_config = config;
}

// Getters
int		ClientHandler::getClientFd() const
{
	return (_client_fd);
}

// Methods
void	ClientHandler::closeConnection(int epoll_fd)
{
	struct epoll_event ev;

	if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, _client_fd, &ev) == -1)
		perror("epoll_ctl: EPOLL_CTL_DEL");
	close(_client_fd);
}

bool	ClientHandler::isRequestComplete(void)
{
	return (_request.isComplete());
}

int	ClientHandler::readData(void)
{
	_request.recv(_client_fd);
	_request.log();
	return (0);
}

int	ClientHandler::send(void)
{
	int	retValue;

	retValue = _response.send(_client_fd);
	while (retValue > 0)
		retValue = _response.send(_client_fd);
	if (retValue == -1)
		return (-1);
	return (0);
}

int	ClientHandler::writeResponse(void)
{
	int	status;

	status = _response.setUp(&_request, _config);
	if (status == CGI_LAUNCHED)
	{
		status = _response.writeToCgi() > 0;
		while (status > 0)
			status = _response.writeToCgi() > 0;
		if (status == -1)
			_response.readCgi(true);
		::usleep(100000);
		while (_response.readCgi(false) > 0) ;
	}
	_response.log();
	send();
	return (0);
}
