/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/28 20:22:00 by rgarrigo          #+#    #+#             */
/*   Updated: 2023/08/11 17:13:00 by motero           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <cstring>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include "HttpRequest.hpp"

#define PORT "8694"
#define BACKLOG 5
#define BUFFER_SIZE 2048


/*
INPUT :	A pointer to an integer representing the socket file descriptor.
OUTPUT :0 if successfully sets the socket for listening, -1 if it fails to bind, 
		2 if there's an error with address information.
DESCRIPTION : This function prepares a socket for accepting connections. 
		It uses TCP over IPv4 (AF_INET and SOCK_STREAM). The port is defined by
		the macro PORT.
		The function iterates over the possible addresses returned by getaddrinfo,
		trying to bind to each one until it succeeds or runs out of options.
QUESTION : Method binds to all addresses returned by getaddrinfo. And not jsut
		the 1st one. It seems to me it will overwrite the socket file descriptor
		each time it binds to a new address.
		Shouldn't it just bind to the first address returned by getaddrinfo?
*/
int	set_and_bind_sock_listen(int *sock_listen)
{
	struct addrinfo	hints;
	struct addrinfo	*addrs;
	int				status;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	status = getaddrinfo(NULL, PORT, &hints, &addrs);
	if (status)
	{
		std::cerr << "getaddrinfo: " << gai_strerror(status) << std::endl;
		return (2);
	}
	for (struct addrinfo *ad = addrs; ad != NULL; ad = ad->ai_next)
	{
		*sock_listen = socket(ad->ai_family, ad->ai_socktype, ad->ai_protocol);
		if (*sock_listen == -1)
			continue ;
		status = bind(*sock_listen, ad->ai_addr, ad->ai_addrlen);
		if (status == -1)
		{
			close(*sock_listen);
			continue ;
		}
	}
	freeaddrinfo(addrs);
	if (*sock_listen == -1)
		return (-1);
	return (0);
}

/*
INPUT : None.
OUTPUT : 0 if everything goes fine, 2 if there's a failure
		in setting up the listening socket.
CHECKS : Checks for failure in set_and_bind_sock_listen function.
DESCRIPTION
		It initializes the server and enters into
		a loop where it continually accepts client connections and
		processes HTTP requests.
		It prints log messages during the connection process and
		delegates the receiving of requests to the HttpRequest class.
		It also sends a response after receiving a complete request.
		The listening socket is configured with a backlog defined by
		the macro BACKLOG.
QUESTION :
		- Shouldn't the server be able to handle multiple connections
		at the same time? It seems to me that it will only handle 1
		connection at a time.

		- There are no way to safe exist the server. It will only stop
		when it receives a SIGINT signal.		
*/
int	main(void)
{
	struct sockaddr_storage	client_addr;
	HttpRequest				request;
	socklen_t				client_addr_size;
	int						sock_listen;
	int						sock_server;
	int						status;

	status = set_and_bind_sock_listen(&sock_listen);
	if (status == -1)
		return (2);
	listen(sock_listen, BACKLOG);
	for (;;)
	{
		client_addr_size = sizeof(client_addr);
		std::cout << "Accept in progress..." << std::endl;
		sock_server = accept(sock_listen, (struct sockaddr *)&client_addr, &client_addr_size);
		for (;;)
		{
			std::cout << "Receiving.." << std::endl;
			request.recv(sock_server);
			if (request.isComplete())
				break ;
		}
		request.respond(sock_server, "200");
		request.clear();
		close(sock_server);
	}
	return (0);
}
