/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientHandler.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/24 23:16:17 by rgarrigo          #+#    #+#             */
/*   Updated: 2023/09/13 16:52:49 by motero           ###   ########.fr       */
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
	_fdClient(other._fdClient),
	_request(other._request),
	_response(other._response)
{
}
ClientHandler&	ClientHandler::operator=(const ClientHandler& other)
{
	if (this != &other) {
		_fdClient = other._fdClient;
		_request = other._request;
		_response = other._response;
		_fdCgiIn = other._fdCgiIn;
		_fdCgiInOpened = other._fdCgiInOpened;
		_fdCgiOut = other._fdCgiOut;
		_fdCgiOutOpened = other._fdCgiOutOpened;
	}
	return *this;
}

// Constructor
ClientHandler::ClientHandler(int fdSock, int fd) :
	_fdClient(fd),
	_fdCgiIn(-1),
	_fdCgiInOpened(false),
	_fdCgiOut(-1),
	_fdCgiOutOpened(false),
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
	newSwitch.mode = mode;
	if (timeout == 0)
		newSwitch.timeout = 0;
	else
		newSwitch.timeout = std::time(NULL) + timeout;
	_epollSwitches.push_back(newSwitch);
	return (0);
}

int	ClientHandler::_setUpResponse(const HttpRequest *request)
{
	int	status;
	int timeout;
	const GlobalConfig& globalConfig = _config.getGlobalConfig();
	
	timeout = globalConfig.timeoutCgi;
	status = _response.setUp(request, _config);
	if (status != CGI_LAUNCHED)
		return (_response.log(), _addSwitch(_fdClient, OUT, timeout), 0);
	_fdCgiIn = _response.getFdCgiIn();
	_fdCgiOut = _response.getFdCgiOut();
	_fdCgiInOpened = true;
	_fdCgiOutOpened = true;
	_addSwitch(_fdCgiIn, OUT, globalConfig.timeoutCgi);
	_addSwitch(_fdCgiOut, IN, globalConfig.timeoutCgi);
	return (0);
}
int	ClientHandler::_readClient(void)
{
	int	status;
	int timeout;
	const GlobalConfig& globalConfig = _config.getGlobalConfig();
	
	timeout = globalConfig.timeoutClient;
	status = _request.recv(_fdClient);
	if (status == -1)
		return (_addSwitch(_fdClient, DEL, 0), -1);
	if (status > 0)
		return (_addSwitch(_fdClient, IN, timeout), 0);
	return (_setUpResponse(&_request));
}
int	ClientHandler::_readCgi(bool timeout)
{
	int	status;
	int	timeoutCgi;
	const GlobalConfig& globalConfig = _config.getGlobalConfig();

	timeoutCgi = globalConfig.timeoutCgi;
	status = _response.readCgi(timeout);
	if (status > 0)
		return (_addSwitch(_fdCgiOut, IN, timeoutCgi), 0);
	_addSwitch(_fdCgiOut, DEL, 0);
	_fdCgiOutOpened = false;
	if (_fdCgiInOpened)
	{
		_addSwitch(_fdCgiIn, DEL, 0);
		_fdCgiInOpened = false;
	}
	_response.log();
	
	int timeOutSend = _config.getGlobalConfig().timeoutClient;
	_addSwitch(_fdClient, OUT, timeOutSend);
	return (0);
}
int	ClientHandler::_readData(int fd)
{
	if (fd == _fdClient)
		return (_readClient());
	if (fd == _fdCgiOut)
		return (_readCgi(false));
	log_message(Logger::ERROR, "Unknown fd to read from: %d", fd);
	return (0);
}

int	ClientHandler::_send(void)
{
	int	status;
	int timeout;
	const t_globalConfig&  globalConfig = _config.getGlobalConfig();

	timeout = globalConfig.timeoutClient;
	status = _response.send(_fdClient);
	if (status == -1)
		return (_clean(), -1);
	if (status == 1)
		return (_addSwitch(_fdClient, OUT, timeout), 1);
	if (status == 0)
		return (_addSwitch(_fdClient, DEL, 0), 0);
	return (0);
}
int	ClientHandler::_writeCgi(void)
{
	int	status;
	int timeout;
	const GlobalConfig& globalConfig = _config.getGlobalConfig();

	timeout = globalConfig.timeoutCgi;
	status = _response.writeToCgi();
	if (status > 0)
		_addSwitch(_fdCgiIn, OUT, timeout);
	_addSwitch(_fdCgiIn, DEL, 0);
	_fdCgiInOpened = false;
	return (0);
}
int	ClientHandler::_writeData(int fd)
{
	if (fd == _fdClient)
		return (_send());
	if (fd == _fdCgiIn)
		return (_writeCgi());
	log_message(Logger::ERROR, "Unknown fd to write to: %d", fd);
	return (0);
}

int	ClientHandler::_manageTimeout(int fd, struct epoll_event &event)
{
	if (fd == _fdClient && (event.events & EPOLLOUT))
		return (_clean(), 0);
	if (fd == _fdClient && (event.events & EPOLLIN))
		return (_setUpResponse(NULL));
	if (fd == _fdCgiIn)
		return (_addSwitch(_fdCgiIn, DEL, 0), 0);
	if (fd == _fdCgiOut)
		return (_readCgi(true));
	return (0);
}

void	ClientHandler::_clean(void)
{
	if (_fdCgiInOpened)
	{
		_addSwitch(_fdCgiIn, DEL, 0);
		_fdCgiInOpened = false;
	}
	if (_fdCgiOutOpened)
	{
		_addSwitch(_fdCgiOut, DEL, 0);
		_fdCgiOutOpened = false;
	}
	_addSwitch(_fdClient, DEL, 0);
}

// Methods
std::vector<t_epollSwitch>	ClientHandler::handleEvent(int fd, struct epoll_event &event, bool timeout)
{
	_epollSwitches.clear();
	if (timeout)
		_manageTimeout(fd, event);
	if (event.events & EPOLLIN)
		_readData(fd);
	if (event.events & EPOLLOUT)
		_writeData(fd);
	if (event.events & EPOLLHUP)
	{
		if (fd == _fdClient)
			_clean();
		if (fd == _fdCgiIn)
			_addSwitch(_fdCgiIn, DEL, 0);
		if (fd == _fdCgiOut)
			_readData(fd);
	}
	return (_epollSwitches);
}
