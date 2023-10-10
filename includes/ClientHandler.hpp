/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientHandler.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/24 23:16:07 by rgarrigo          #+#    #+#             */
/*   Updated: 2023/09/20 19:01:26 by rgarrigo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENTHANDLER_HPP
# define CLIENTHANDLER_HPP


# include <iostream>
# include <unistd.h>
# include <sys/epoll.h>
# include <vector>
# include "Config.hpp"
# include "HttpRequest.hpp"
# include "HttpResponse.hpp"
# include "Logger.hpp"

typedef enum e_epollMode
{
	IN,
	OUT,
	IN_OUT,
	DEL
}	t_epollMode;

typedef struct s_epollSwitch
{
	int			fd;
	t_epollMode	mode;
	std::time_t	timeout;
}	t_epollSwitch;

class ClientHandler {
	private:
		int								_fdClient;
		int								_fdCgiIn;
		bool							_fdCgiInOpened;
		int								_fdCgiOut;
		bool							_fdCgiOutOpened;
		HttpRequest						_request;
		HttpResponse					_response;
		std::vector<t_epollSwitch>		_epollSwitches;

	// Static
		static Config					_config;
		static std::map<int, uint16_t>	_port;

	// Utils
		int		_addSwitch(int fd, t_epollMode mode);
		int		_manageTimeout(int fd, struct epoll_event &event);
		int		_readClient(void);
		int		_readCgi(bool timeout);
		int		_readData(int fd);
		int		_send(void);
		int		_setUpResponse(const HttpRequest *request);
		int		_writeCgi(void);
		int		_writeData(int fd);
		void	_clean(void);
		void	_reset(void);

	public:
	// Coplien
		ClientHandler();
		ClientHandler(int fdSock, int fd);
		ClientHandler(const ClientHandler& other);
		ClientHandler& operator=(const ClientHandler& other);
		~ClientHandler();

	// Setters
		static void	setConfig(const Config &config);
		static void	addPort(int fdSock, uint16_t port);

	//Getters
		int				getFdClient(void) const;
		int				getFdCgiIn(void) const;
		int				getFdCgiOut(void) const;
		bool			getFdCgiInOpened(void) const;
		bool			getFdCgiOutOpened(void) const;


	//Methods
		std::vector<t_epollSwitch>	handleEvent(int fd, struct epoll_event &event, bool timeout);
};

std::ostream& operator<<(std::ostream& os, const ClientHandler & other);

#endif
