/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientHandler.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgarrigo <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/24 23:16:17 by rgarrigo          #+#    #+#             */
/*   Updated: 2023/09/01 23:44:17 by rgarrigo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ClientHandler.hpp"

Config				ClientHandler::_config;
std::map<int, uint16_t>	ClientHandler::_port;

ClientHandler::~ClientHandler() 
{
}

//constructor by copy
ClientHandler::ClientHandler(void)
{
}
ClientHandler::ClientHandler(const ClientHandler& other) : 
	_client_fd(other._client_fd),
	_request(other._request),
	_response(other._response)
{
}

ClientHandler::ClientHandler(int fdSock, int fd) :
	_client_fd(fd),
	_request(),
	_response(_port[fdSock])
{
}

// = operator overload
ClientHandler& ClientHandler::operator=(const ClientHandler& other) {
	if (this != &other) {
		_client_fd = other._client_fd;
		_request = other._request;
		_response = other._response;
	}
	return *this;
}

void    ClientHandler::readData() {
	_request.recv(_client_fd);
}

int     ClientHandler::getClientFd() const {
	return _client_fd;
}


void	ClientHandler::addPort(int fdSock, uint16_t port)
{
	(ClientHandler::_port)[fdSock] = port;
}
void	ClientHandler::setConfig(const Config &config)
{
	ClientHandler::_config = config;
}

// Write the response back to the client.
void    ClientHandler::writeResponse() {
	_response.setUp(&_request, _config);
	_response.send(_client_fd);
}

bool    ClientHandler::isRequestComplete() {
	if (_request.isComplete()) {
		return true;
	}
	return false;
}

void    ClientHandler::closeConnection(int epoll_fd) {
	struct epoll_event ev;
	if(epoll_ctl(epoll_fd, EPOLL_CTL_DEL, _client_fd, &ev) == -1) {
		perror("epoll_ctl: EPOLL_CTL_DEL");
		// Handle error
	}
	close(_client_fd);
}
