/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgarrigo <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/28 20:22:00 by rgarrigo          #+#    #+#             */
/*   Updated: 2023/07/29 18:51:06 by rgarrigo         ###   ########.fr       */
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

int	main(void)
{
	int						sock_listen;
	int						sock_server;
	struct sockaddr_storage	client_addr;
	socklen_t				client_addr_size;
	int						status;
	HttpRequest				request;

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
