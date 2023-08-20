/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/03/11 01:18:42 by rgarrigo          #+#    #+#             */
/*   Updated: 2023/08/18 22:28:36 by motero           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

//to make a define with this
const int MAX_EVENTS = 10;  // Number of maximum events to be returned by epoll_wait to transfom in a define

volatile sig_atomic_t Server::run = true; // Initialize the static member

Server::Server() : 
    epoll_fd(-1),
	defaultConf("./config/default.conf")
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
int	Server::setUpSocket(int* sock_listen, const std::string& port)
{
	struct addrinfo	hints;
	struct addrinfo	*addrs;
	int				status;
	
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	status = getaddrinfo(NULL, port.c_str(), &hints, &addrs);
	if (status) {
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
		std::cout << "Socket created for port: " << port << std::endl;
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
		std::cout << "Bind successful for port: " << port << std::endl;

	}
	freeaddrinfo(addrs);
	if (*sock_listen == -1)
		return (-1);
	return (0);
}

int Server::setUpEpoll() {
	int epoll_fd = epoll_create(2);
	if (epoll_fd == -1) {
		return perror("epoll_create1"), -1;
	}

	for (std::vector<int>::iterator it = sock_listens.begin();
		it != sock_listens.end(); ++it) {
		
		struct epoll_event ev;
		ev.events = EPOLLIN;
		ev.data.fd = *it;
		
		if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, *it, &ev) == -1) {
			return perror(strerror(errno)), -1;
		}
	}
	return epoll_fd;
}

int Server::accept_new_client(int epoll_fd, int sock_listen) {
	
	struct sockaddr_storage	client_addr;
	struct epoll_event 		event;
	socklen_t 				client_addr_size;
	int 					sock_server;
	
	std::cout << "inside accept new client" << std::endl;
	client_addr_size = sizeof(client_addr);
	sock_server = accept(sock_listen, (struct sockaddr *)&client_addr, &client_addr_size);
	if (sock_server == -1) {
		std::cout << "accept failed" << std::endl;
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
	
	// HttpRequestBase request;
	
	if (ongoingRequests.find(client_fd) == ongoingRequests.end()) {
		// New client, create a HttpRequestBase for it
		ongoingRequests[client_fd] = HttpRequestBase();
	}

	HttpRequestBase& request = ongoingRequests[client_fd];
	
	try {
		request.recv(client_fd);
	} catch (const std::exception& e) {
		std::cerr << e.what() << '\n';
		close_and_cleanup(epoll_fd, client_fd);
		ongoingRequests.erase(client_fd);
		throw;  // return would be beter	
	} 

	if (request.isComplete()) {
		HttpRequestBase *NewReqObj = request.createRequestObj(request._method);
		// std::cout << "before respond" << std::endl;
		NewReqObj->respond(client_fd, "200");
		std::cout << "before clear" << std::endl;
		delete NewReqObj;
		request.clear();
		close_and_cleanup(epoll_fd, client_fd);
		ongoingRequests.erase(client_fd);
	}
	else {
		close_and_cleanup(epoll_fd, client_fd);
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

int Server::handle_epoll_events(int epoll_fd) {
	
	struct epoll_event	events[MAX_EVENTS];
	HttpRequestBase		request;
	int					num_fds;
	int					client_fd;

	int timeout = calculate_dynamic_timeout();

	std::cout << "Waiting for an epoll event..." << std::endl;
	num_fds = epoll_wait(epoll_fd, events, MAX_EVENTS, timeout);
	if (num_fds == -1) {
		if (errno == EINTR) {
			return perror("epoll_wait"), -1;
		}
		return perror("epoll_wait"), -1;
	}
	std::cout << "Epoll returned with " << num_fds << " events." << std::endl;
	if (num_fds == 0) {
		std::cout << "Waiting for new clients or data from existing clients..." << std::endl;
		return 1;
	}

	for (int i = 0; i < num_fds; i++) {
        inspect_epoll_event(events[i].events);

		if (std::find(	sock_listens.begin(),
						sock_listens.end(),
						events[i].data.fd) != sock_listens.end()) {
			std::cout << "inside for loop" << std::endl;
			client_fd = accept_new_client(epoll_fd, events[i].data.fd);
			std::cout << "After" << std::endl;
			if (client_fd == -1) {
				if (errno == EAGAIN || errno == EWOULDBLOCK) {
					// We have processed all incoming connections.
					break;
				} else {
					perror("accept");
					break;
				}
			}
			handle_client_data(epoll_fd, client_fd);
			std::cout << "Accepted new client with fd: " << client_fd << " on port: " << events[i].data.fd << std::endl;
		} else {
			handle_client_data(epoll_fd, events[i].data.fd);
		}
		std::cout << "outside while loop" << std::endl;
	}
	return 0;
}

void Server::inspect_epoll_event(uint32_t events) {
    // Check and print the type of event
    if (events & EPOLLIN) std::cout << "EPOLLIN Event" << std::endl;
    if (events & EPOLLOUT) std::cout << "EPOLLOUT Event" << std::endl;
    if (events & EPOLLERR) std::cout << "EPOLLERR Event" << std::endl;
    if (events & EPOLLHUP) std::cout << "EPOLLHUP Event" << std::endl;
    
}

void	Server::signal_handler(int sig)
{
    if (sig == SIGINT)
        Server::run = false;
}

//This method will probably dissapear or change completely, just here to bootstrap 
// multiple ports
std::vector<std::string> Server::getPorts() {
	
	std::vector<std::string> ports;
	ports.push_back(PORTAL);
	ports.push_back(PORT);
	
	return ports;
}

void Server::start() {
    run = true;
    signal(SIGINT, signal_handler); // Register signal handler
	
	// Loop over ports
	std::vector<std::string> ports = getPorts();
	for (std::vector<std::string>::const_iterator it = ports.begin();
		it != ports.end();
		++it) {
		int socket;
		if (setUpSocket(&socket, *it) == -1) {
			std::cerr << "Failed to set up socket at port" <<  socket << std::endl;
			continue ;
		}
		sock_listens.push_back(socket);
    	if (listen(socket, BACKLOG) == -1) {
			perror("listen");
			return ;
		}
		std::cout << "Listening on port: " << *it << std::endl;
	}

    epoll_fd = setUpEpoll();
	
    if (epoll_fd == -1) {
	    std::cerr << "Failed to set up epoll" << std::endl;
        return;
    }
    while (run) {
        if (handle_epoll_events(epoll_fd) == -1)
            break;
    }

	close(epoll_fd);
	for (std::vector<int>::iterator it = sock_listens.begin();
		it != sock_listens.end();
		++it) {
			close(*it);
	}
}

void Server::stop() {
    run = false;

	for (std::vector<int>::iterator it = sock_listens.begin();
		it != sock_listens.end();
		++it) {
		close(*it);
	}
	
    if (epoll_fd != -1) {
        close(epoll_fd);
    }
}

void Server::loadDefaultConfig() {
	std::ifstream	file;

	file.open(defaultConf.c_str(), std::fstream::in);
	if (!file) {
		std::cerr << "Config: error: failed to open default config file." << std::endl;
		exit(EXIT_FAILURE);
	}
	if (!Config::checkNorm(file))
		exit(EXIT_FAILURE);
	_config = Config();
	_config.setupConf(file);
	std::cout << "Config = " << defaultConf << std::endl;
	
}

void Server::loadConfig(const std::string& configPath) {
    // Load server configuration from the given path.
    // create a specific object with a new class
    
	std::ifstream	file;

	file.open(configPath.c_str(), std::fstream::in);
	if (!file) {
		std::cerr << "Config: error: failed to open config file, using default config instead." << std::endl;
		loadDefaultConfig();
		return ;
	}
	if (!Config::checkNorm(file)) {
		loadDefaultConfig();
		return ;
	}
	_config = Config();
	_config.setupConf(file);
	std::cout << "Config = " << configPath << std::endl;

    return ;
}

void Server::close_and_cleanup(int epoll_fd, int client_fd) {
    struct epoll_event ev;
    if(epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, &ev) == -1) {
        perror("epoll_ctl: EPOLL_CTL_DEL");
        // Handle error
    }
    close(client_fd);
}

std::ostream& operator<<(std::ostream& os, const Server & server);


