/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   clientHandler.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: znogueir <znogueir@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/24 23:16:17 by rgarrigo          #+#    #+#             */
/*   Updated: 2023/10/01 21:12:49 by rgarrigo         ###   ########.fr       */
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

//getters

int	ClientHandler::getFdClient(void) const
{
	return (_fdClient);
}

int	ClientHandler::getFdCgiIn(void) const
{
	return (_fdCgiIn);
}

int ClientHandler::getFdCgiOut(void) const
{
	return (_fdCgiOut);
}

bool ClientHandler::getFdCgiInOpened(void) const
{
	return (_fdCgiInOpened);
}

bool ClientHandler::getFdCgiOutOpened(void) const
{
	return (_fdCgiOutOpened);
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
int	ClientHandler::_addSwitch(int fd, t_epollMode mode)
{
	t_epollSwitch	newSwitch;

	newSwitch.fd = fd;
	newSwitch.mode = mode;
	if (fd == _fdCgiIn && mode == DEL)
		_fdCgiInOpened = false;
	if (fd == _fdCgiOut && mode == DEL)
		_fdCgiOutOpened = false;
	newSwitch.timeout = 0;
	if (mode != DEL)
	{
		if (fd == _fdClient)
			newSwitch.timeout = std::time(NULL) + _config.getTimeoutClient();
		if (fd == _fdCgiIn || fd == _fdCgiOut)
			newSwitch.timeout = std::time(NULL) + _config.getTimeoutCgi();
	}
	_epollSwitches.push_back(newSwitch);
	return (0);
}

int	ClientHandler::_setUpResponse(const HttpRequest *request)
{
	int					status;

	status = _response.setUp(request, _config);

	// log_message(Logger::WARN, "STATUS IN SETUP : %d", status);
	if (status != CGI_LAUNCHED)
		return (_response.log(), _addSwitch(_fdClient, OUT), 0);

	_fdCgiIn = _response.getFdCgiIn();
	_fdCgiOut = _response.getFdCgiOut();

	_fdCgiInOpened = true;
	_fdCgiOutOpened = true;

	_addSwitch(_fdCgiIn, OUT);
	_addSwitch(_fdCgiOut, IN);

	return (0);
}

int	ClientHandler::_readClient(void)
{
	int	status;

	status = _request.recv(_fdClient);
	if (status == -1)
		return (log_message(Logger::DEBUG, "RECV status -1") ,_addSwitch(_fdClient, DEL), -1);

	// log_message(Logger::WARN, "STATUS IN READ : %d", status);
	if (status > 0)
		return (_addSwitch(_fdClient, IN), 0);

	return (_setUpResponse(&_request));
}

int	ClientHandler::_readCgi(bool timeout)
{
	int	status;

	status = _response.readCgi(timeout);
	if (status > 0)
		return (_addSwitch(_fdCgiOut, IN), 0);

	_addSwitch(_fdCgiOut, DEL);
	if (_fdCgiInOpened)
		_addSwitch(_fdCgiIn, DEL);
	_response.log();
	
	_addSwitch(_fdClient, OUT);
	return (0);
}

int	ClientHandler::_readData(int fd)
{
	log_message(Logger::DEBUG, "Reading from fd %d", fd);
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

	status = _response.send(_fdClient);
	if (status == -1)
		return (_clean(), -1);
	if (status == 1)
		return (_addSwitch(_fdClient, OUT), 1);
	if (status == 0)
		return (_reset(), _addSwitch(_fdClient, IN), 0);
	return (0);
}

int	ClientHandler::_writeCgi(void)
{
	int	status;

	status = _response.writeToCgi();
	if (status > 0)
		_addSwitch(_fdCgiIn, OUT);
	_addSwitch(_fdCgiIn, DEL);
	return (0);
}

int	ClientHandler::_writeData(int fd)
{
	log_message(Logger::DEBUG, "Writing to fd %d", fd);
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
	if (fd == _fdClient && event.events == 0)
		return (_clean(), 0);
	if (fd == _fdCgiIn)
		return (_addSwitch(_fdCgiIn, DEL), 0);
	if (fd == _fdCgiOut)
	{
		if (_fdCgiInOpened)
			_addSwitch(_fdCgiIn, DEL);
		return (_addSwitch(_fdCgiOut, DEL), _readCgi(true));
	}
	return (0);
}

void	ClientHandler::_reset(void)
{
	if (_fdCgiInOpened)
		_addSwitch(_fdCgiIn, DEL);
	if (_fdCgiOutOpened)
		_addSwitch(_fdCgiOut, DEL);
	_fdCgiIn = 0;
	_fdCgiOut = 0;
	_request = HttpRequest();
	_response.clean();
}

void	ClientHandler::_clean(void)
{
	if (_fdCgiInOpened)
		_addSwitch(_fdCgiIn, DEL);
	if (_fdCgiOutOpened)
		_addSwitch(_fdCgiOut, DEL);
	_addSwitch(_fdClient, DEL);
}

// Methods
std::vector<t_epollSwitch>	ClientHandler::handleEvent(int fd, struct epoll_event &event, bool timeout)
{
	_epollSwitches.clear();
	if ((event.events & EPOLLERR) || (event.events & EPOLLHUP))
	{
		log_message(Logger::DEBUG, "EPOLLERR or EPOLLHUP on fd %d", fd);
		if (fd == _fdClient)
			_clean();
		if (fd == _fdCgiIn)
			_addSwitch(_fdCgiIn, DEL);
		if (fd == _fdCgiOut)
			_readData(fd);
		return (_epollSwitches);
	}
	if (timeout)
		_manageTimeout(fd, event);
	if (event.events & EPOLLIN)
		_readData(fd);
	if (event.events & EPOLLOUT)
		_writeData(fd);
	return (_epollSwitches);
}
