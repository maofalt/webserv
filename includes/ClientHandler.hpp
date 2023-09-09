/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientHandler.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/24 23:16:07 by rgarrigo          #+#    #+#             */
/*   Updated: 2023/09/09 18:14:03 by motero           ###   ########.fr       */
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
	t_epollMode	mode;
	int			fd;
	std::time_t	timeout;
}	t_epollSwitch;

class ClientHandler {
	private:
		int																	_fdClient;
		int																	_fdCgiIn;
		int																	_fdCgiOut;
		HttpRequest															_request;
		HttpResponse														_response;
		static Config														_config;
		static std::map<int, uint16_t>										_port;
		std::vector<t_epollSwitch>											_epollSwitches;

	// Utils

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

	//Methods
		std::vector<t_epollSwitch>	handleEvent(int fd, struct epoll_event &event);
};

std::ostream& operator<<(std::ostream& os, const ClientHandler & other);

#endif
