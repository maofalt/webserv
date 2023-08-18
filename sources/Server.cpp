/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/03/11 01:18:42 by rgarrigo          #+#    #+#             */
/*   Updated: 2023/08/17 18:19:48 by motero           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

//to make a define with this
const int MAX_EVENTS = 10;  // Number of maximum events to be returned by epoll_wait to transfom in a define

volatile sig_atomic_t Server::run = true; // Initialize the static member

Server::Server() : 
    epoll_fd(-1),
    sock_listens(1,-1)
{
}

Server::~Server() {
    stop();
}

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
int Server::setUpSocket(int* sock_listen) {
	return set_and_bind_sock_listen(sock_listen);
}

int	Server::set_and_bind_sock_listen(int *sock_listen)
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
		//set SO_REUSEADDR option, but this make provoke to hijack the sokt it we launch the server serveral times!!
		//we have to find a turnaround to fix this!! 
		int optval = 1;
		if (setsockopt(*sock_listen, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
			close(*sock_listen);
			return perror(strerror(errno)), -1;
		}
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

int Server::setUpEpoll(int sock_listen) {
	int epoll_fd = epoll_create(1);
	if (epoll_fd == -1) {
		return perror("epoll_create1"), -1;
	}

	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = sock_listen;

	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock_listen, &ev) == -1) {
		return perror(strerror(errno)), -1;
	}

	return epoll_fd;
}

int Server::accept_new_client(int epoll_fd, int sock_listen) {
	
	struct sockaddr_storage	client_addr;
	struct epoll_event 		event;
	socklen_t 				client_addr_size;
	int 					sock_server;
	
	client_addr_size = sizeof(client_addr);
	sock_server = accept(sock_listen, (struct sockaddr *)&client_addr, &client_addr_size);
	if (sock_server == -1) {
		perror("accept");
		return -1;
	}

	std::cout << "New client connected on descriptor " << sock_server << "!" << std::endl;
	
	event.events = EPOLLIN;
	event.data.fd = sock_server;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock_server, &event) == -1) {
		perror("epoll_ctl: sock_server");
		close(sock_server);
		return -1;
	}
	return sock_server;
}

void Server::handle_client_data(int epoll_fd, int client_fd) {
	
	
	(void)epoll_fd; //we should use this variable to handle the error
	std::cout << "Receiving data from client on descriptor " << client_fd << std::endl;
	
	
	if (ongoingRequests.find(client_fd) == ongoingRequests.end()) {
		// New client, create a HttpRequest for it
		ongoingRequests[client_fd] = HttpRequest();
	}

	HttpRequest& request = ongoingRequests[client_fd];
	
	try {
		request.recv(client_fd);
	} catch (const std::exception& e) {
		std::cerr << e.what() << '\n';
		close(client_fd);
		ongoingRequests.erase(client_fd);
		throw;  // return would be beter	
	}

	if (request.isComplete()) {
		std::cout << "before respond" << std::endl;
		request.respond(client_fd, "200");
		std::cout << "before clear" << std::endl;
		request.clear();
		close(client_fd);
		ongoingRequests.erase(client_fd);
	}
	else {
		std::cout << "Request not complete yet" << std::endl;
	}
}

//if we want to ahve a more robust logi for time out
// if we know that the server is under heavy load, 
// might opt for a longer timeout, and during idle times, a shorter one.
/*
Nginx : 
Nginx often sets the epoll_wait timeout dynamically based on various factors. 
If there are immediate tasks to handle, it might set the timeout to 0 to poll; 
if there are delayed tasks, it will calculate the time until the nearest timer 
and use that as the timeout. If there's nothing immediate to do, it might block 
indefinitely.
*/
int Server::calculate_dynamic_timeout() {
	// Logic to determine appropriate timeout
	int timeout_value = 1000;
	return timeout_value;
}

int Server::handle_epoll_events(int epoll_fd, int sock_listen) {
	
	struct epoll_event	events[MAX_EVENTS];
	HttpRequest			request;
	int					num_fds;
	int					sock_server;

	int timeout = calculate_dynamic_timeout();

	num_fds = epoll_wait(epoll_fd, events, MAX_EVENTS, timeout);
	if (num_fds == -1) {
		if (errno == EINTR) {
			return perror("epoll_wait"), -1;
		}
		return perror("epoll_wait"), -1;
	}

	if (num_fds == 0) {
		std::cout << "Waiting for new clients or data from existing clients..." << std::endl;
		return 1;
	}

	for (int i = 0; i < num_fds; i++) {
		if (events[i].data.fd == sock_listen) {
			sock_server = accept_new_client(epoll_fd, sock_listen);
			if (sock_server == -1) {
				continue;
			}
		} else {
			handle_client_data(epoll_fd, events[i].data.fd);
		}
	}
	return 0;
}

void	Server::signal_handler(int sig)
{
    if (sig == SIGINT)
        Server::run = false;
}

void Server::start() {
    run = true;
    signal(SIGINT, signal_handler); // Register signal handler
    if (setUpSocket(&sock_listens[0]) == -1) {
        std::cerr << "Failed to set up socket." << std::endl;
        return;
    }
    listen(sock_listens[0], BACKLOG);
    epoll_fd = setUpEpoll(sock_listens[0]);
    if (epoll_fd != -1) {
        while (run) {
            if (handle_epoll_events(epoll_fd, sock_listens[0]) == -1)
                break;
        }
        close(epoll_fd);
    }
    close(sock_listens[0]);
}

void Server::stop() {
    run = 0;
    close(sock_listens[0]);
    if (epoll_fd != -1) {
        close(epoll_fd);
    }
}
    

void Server::loadConfig(const std::string& configPath) {
    // Load server configuration from the given path.
    //create a specific bobject witha new class
    (void)configPath;
    return ;
}

std::ostream& operator<<(std::ostream& os, const Server & server);


