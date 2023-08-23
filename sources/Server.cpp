/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/03/11 01:18:42 by rgarrigo          #+#    #+#             */
/*   Updated: 2023/08/23 17:12:36 by motero           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

volatile sig_atomic_t Server::run = true; // Initialize the static member

Server::Server() : 
    epoll_fd(-1),
	defaultConf("./config/default.conf")
{
}

Server::~Server() {
    stop();
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

int Server::initializeSocket(const addrinfo* ad,
							int* sock_listen,
							const std::string& port) {

	int optval = 1;
							
    *sock_listen = socket(ad->ai_family, ad->ai_socktype, ad->ai_protocol);
    if (*sock_listen == -1) {
        std::cerr << "Socket error: " << strerror(errno) << std::endl;
        return -1;
    }

    std::cout << "Socket created for port: " << port << std::endl;
    if (setsockopt(*sock_listen, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
        std::cerr << "Setsockopt error: " << strerror(errno) << std::endl;
        close(*sock_listen);
        return -1;
    }

    if (bind(*sock_listen, ad->ai_addr, ad->ai_addrlen) == -1) {
        std::cerr << "Bind error: " << strerror(errno) << std::endl;
        close(*sock_listen);
        return -1;
    }

    std::cout << "Bind successful for port: " << port << std::endl;
    return 0;
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
*/
int Server::setUpSocket(int* sock_listen, const std::string& port) {
    addrinfo hints, *addrs, *ad;
    int status;

    // Setup addrinfo structure
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags |= AI_PASSIVE;

    status = getaddrinfo(NULL, port.c_str(), &hints, &addrs);
    if (status) {
        std::cerr << "getaddrinfo: " << gai_strerror(status) << std::endl;
        return 2;
    }

	// Iterate over possible addresses and bind to the first one that works
    for (ad = addrs; ad != NULL; ad = ad->ai_next) {
        if (initializeSocket(ad, sock_listen, port) == 0) {
            break;
        }
    }

    freeaddrinfo(addrs);
	
	// If ad is NULL, then we failed to bind to any address
    if (ad == NULL) {
        std::cerr << "Failed to bind to any address" << std::endl;
        return -1;
    }

    return 0;
}


/**
 * Set up epoll.
 * @return epoll file descriptor.
 */
int Server::setUpEpoll() {
	// Create epoll
	int epoll_fd = epoll_create(2);
	if (epoll_fd == -1) {
		return perror("epoll_create1"), -1;
	}
	
	// Add each socket to epoll
	for (std::vector<int>::iterator it = sock_listens.begin();
		it != sock_listens.end(); ++it) {
		
		// Set up epoll event struct and set it to for read events
		struct epoll_event ev;
		ev.events = EPOLLIN;
		ev.data.fd = *it;
		
		// Add socket to epoll
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
	
	client_addr_size = sizeof(client_addr);
	sock_server = accept(sock_listen, (struct sockaddr *)&client_addr, &client_addr_size);
	if (sock_server == -1) {
		perror("accept");
		return -1;
	}

	std::cout << "New client connected on descriptor " << sock_server << "!" << std::endl;
	
	//add to Epoll
	event.events = EPOLLIN;
	event.data.fd = sock_server;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock_server, &event) == -1) {
		perror("epoll_ctl: sock_server");
		close(sock_server);
		return -1;
	}

	//add to clientHandlers
	clientHandlers[sock_server] = ClientHandler(sock_server, HttpRequestBase());
	return sock_server;
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

	int timeout = calculate_dynamic_timeout();

	std::cout << "Waiting for an epoll event..." << std::endl;
	num_fds = epoll_wait(epoll_fd, events, MAX_EVENTS, timeout);
	if (num_fds == -1) {
		if (errno == EINTR) { return perror("epoll_wait"), -1; }
		return perror("epoll_wait"), -1;
	}
	
	std::cout << "Epoll returned with " << num_fds << " events." << std::endl;
	
	for (int i = 0; i < num_fds; i++) {
        inspect_epoll_event(events[i].events);
		
		// Check if the event's fd is a listening socket
		if (std::find(	sock_listens.begin(),
						sock_listens.end(),
						events[i].data.fd) != sock_listens.end()) {
			if (accept_new_client(epoll_fd, events[i].data.fd)== -1) {
				if (errno == EAGAIN || errno == EWOULDBLOCK) { break; }
				perror("accept"); }
			continue;
		} 
		// Else it's a client socket
		std::cout << "Handling client " << events[i].data.fd << "event" << std::endl;
		handleClientEvent(epoll_fd, events[i]);
			
	}
	return 0;
}

int		Server::handleClientEvent(int epoll_fd, struct epoll_event& event) {

	int client_fd = event.data.fd;
	
	if (clientHandlers.find(client_fd) == clientHandlers.end()) {
    	std::cerr << "Unknown client fd: " << client_fd << std::endl;
    	return 1;
	}
	
	ClientHandler& client = clientHandlers[client_fd];
	std::cout << "Client " << client_fd << "exists!!" << std::endl;

	 inspect_epoll_event(event.events);

	// Depending on the epoll event, decide the action on the client
	if (event.events & EPOLLIN) {
		try {
    		std::cout << "Reading data" << std::endl;
			client.readData();
			std::cout << "Data read" << std::endl;
		} catch (const std::exception& e) {
			close_and_cleanup(epoll_fd, client_fd);
			clientHandlers.erase(client_fd); 
			throw;
		}
    	if (client.isRequestComplete()) {
			std::cout << "Request complete" << std::endl;
    		if (changeClientEpollMode(epoll_fd, client_fd, EPOLLOUT) != 0) {
				client.closeConnection(epoll_fd);
				clientHandlers.erase(client_fd);  // remove ClientHandler for this client
			}
			std::cout << "Changed epoll mode to EPOLLOUT" << std::endl;
    	}
    }
    else if (event.events & EPOLLOUT) {
		std::cout << "Writing response" << std::endl;
        client.writeResponse();
		std::cout << "Response written" << std::endl;
		close_and_cleanup(epoll_fd, client_fd);
		clientHandlers.erase(client_fd); 
    }
    else if (event.events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
        std::cerr << "Error on client fd: " << client_fd << std::endl;
        client.closeConnection(epoll_fd);
        clientHandlers.erase(client_fd);  // remove ClientHandler for this client
    }
	return 0;
}

int	Server::changeClientEpollMode(int epoll_fd, int client_fd, int mode) {
	
	struct epoll_event ev;
	ev.events = mode;
	ev.data.fd = client_fd;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_fd, &ev) == -1) {
		perror("epoll_ctl: EPOLL_CTL_MOD");
		return -1;
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
	_config = Config();
	if (_config.setupConf(file, defaultConf)) {
		exit(EXIT_FAILURE);
	}
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
	_config = Config();
	if (_config.setupConf(file, configPath)) {
		loadDefaultConfig();
		return ;
	}
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


