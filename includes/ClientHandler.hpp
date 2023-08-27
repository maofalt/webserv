/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientHandler.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgarrigo <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/24 23:16:07 by rgarrigo          #+#    #+#             */
/*   Updated: 2023/08/26 13:15:34 by rgarrigo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENTHANDLER_HPP
# define CLIENTHANDLER_HPP


# include <iostream>
# include <unistd.h>
# include <sys/epoll.h>
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
		int				_client_fd;
		HttpRequest		_request;
		HttpResponse	_response;

	public:
		ClientHandler();
		ClientHandler(uint16_t port, int fd);
		ClientHandler(const ClientHandler& other);
		ClientHandler& operator=(const ClientHandler& other);
	// Destructor
		~ClientHandler();

	// Getters
		HttpRequest getRequest() const;
		int getClientFd() const;

	//Methods
		void readData();
		void writeResponse();
		bool isRequestComplete();
		void closeConnection(int epoll_fd);
};

std::ostream& operator<<(std::ostream& os, const ClientHandler & other);

#endif
