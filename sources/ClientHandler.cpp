/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientHandler.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/24 23:16:17 by rgarrigo          #+#    #+#             */
/*   Updated: 2023/09/09 19:27:54 by rgarrigo         ###   ########.fr       */
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

// Utils
int	ClientHandler::_addSwitch(int fd, t_epollMode mode, std::time_t timeout)
{
	t_epollSwitch	newSwitch;

	newSwitch.fd = fd;
	newSitch.mode = mode;
	if (timeout == 0)
		newSwitch.timeout = 0;
	else
		newSwitch.timeout = std::time() + timeout;
	_epollSwitches.push_back(newSwitch);
	return (0);
}

int	ClientHandler::_readClient(void)
{
	int	status;

	status = _response.recv();
	if (status == -1)
		return (_addSwitch(_clientFd, DEL, 0), -1)
	if (status > 0)
		return (_addSwitch(_clientFd, IN, TIMEOUT_RECV), 0);
	_response.setUp(&_request, _config);
	if (status != CGI_LAUNCHED)
		return (_response.log(), _addSwitch(_clientFd, OUT, TIMEOUT_SEND), 0);
	_fdCgiIn = _response.getFdCgiIn();
	_fdCgiOut = _response.getFdCgiOut();
	_fdCgiInOpened = true;
	_fdCgiOutOpened = true;
	_addSwitch(_fdCgiIn, OUT, TIMEOUT_CGI_IN);
	_addSwitch(_fdCgiOut, IN, TIMEOUT_CGI_OUT);
	return (0);
}
int	ClientHandler::_readCgi(void)
{
	int	status;

	status = _response.readCgi(false);
	if (status > 0)
		return (_addSwitch(_fdCgiOut, IN, TIMEOUT_CGI_OUT), 0);
	_fdCgiOutOpened = false;
	if (_fdCgiInOpened)
		_addSwitch(_fdCgiIn, DEL, 0);
	_response.log();
	_addSwitch(_fdClient, OUT, TIMEOUT_SEND);
	return (0);
}
int	ClientHandler::_readData(int fd)
{
	if (fd == _fdClient)
		return (_readClient());
	if (fd == _fdCgiOut)
		return (_readCgi());
	return (0);
}

int	ClientHandler::_send(void)
{
	int	status;

	status = _response.send(_fdClient);
	if (status == -1)
		return (_clean(), -1);
	if (status == 1)
		return (_addSwitch(_fdClient, IN, TIMEOUT_SEND), 1);
	return (0);
}
int	ClientHandler::_writeCgi(void)
{
	int	status;

	status = _response.writeToCgi();
	if (status > 0)
		_addSwitch(_fdCgiIn, OUT, TIMEOUT_CGI_IN);
	_fdCgiInOpened = false;
	return (0);
}
int	ClientHandler::_writeData(void)
{
	if (fd == _fdClient)
		return (_send());
	if (fd == _fdCgiIn)
		return (_writeCgi());
	return (0);
}

void	ClientHandler::_clean(void)
{
	if (_fdCgiInOpened)
		_addSwitch(_fdCgiIn, DEL, 0);
	if (_fdCgiOutOpened)
		_addSwitch(_fdCgiOut, DEL, 0);
	close(_fdClient);
}

// Methods
std::vector<t_epollSwitch>	ClientHandler::handleEvent(int fd, struct epoll_event &event)
{
	_epollSwitches.clear();
	if (event.events & EPOLLIN)
		_readData(fd);
	if (event.events & EPOLLOUT)
		_writeData(fd);
	return (_epollSwicthes);
}

std::vector<int>	ClientHandler::getOpenedFd(void) const
{
	std::vector <int>	fd;

	fd.push_back(_fdClient);
	return (fd);
}
