/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientHandler.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgarrigo <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/24 23:16:07 by rgarrigo          #+#    #+#             */
/*   Updated: 2023/09/02 21:35:10 by rgarrigo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENTHANDLER_HPP
# define CLIENTHANDLER_HPP


# include <iostream>
# include <unistd.h>
# include <sys/epoll.h>
# include "Config.hpp"
# include "HttpRequest.hpp"
# include "HttpResponse.hpp"
# include "Logger.hpp"

/*
   ClientHandler Class:

Purpose:    To manage communication with an individual client.
Attributes:
	- Socket or file descriptor for the client.
	- An instance of HttpRequest (or later, HttpRequest 
and derived classes) to parse the incoming request.
	- An instance of HttpResponse to prepare and send the response.
Methods:
	- Read from the client.
	- Write to the client.
	- Disconnect/close client connection.
 */

class ClientHandler {
	private:
		int								_client_fd;
		HttpRequest						_request;
		HttpResponse					_response;
		static Config					_config;
		static std::map<int, uint16_t>	_port;

	public:
		ClientHandler();
		ClientHandler(int fdSock, int fd);
		ClientHandler(const ClientHandler& other);
		ClientHandler& operator=(const ClientHandler& other);
	// Destructor
		~ClientHandler();

	// Getters
		HttpRequest getRequest() const;
		int			getClientFd() const;
	
	// Setters
		static void	setConfig(const Config &config);
		static void	addPort(int fdSock, uint16_t port);

	//Methods
		int readData(void);
		int	send(void);
		int writeResponse(void);
		bool isRequestComplete(void);
		void closeConnection(int epoll_fd);
};

std::ostream& operator<<(std::ostream& os, const ClientHandler & other);

#endif
