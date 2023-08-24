/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/03/11 01:18:42 by rgarrigo          #+#    #+#             */
/*   Updated: 2023/08/24 18:05:31 by motero           ###   ########.fr       */
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


/**
 * @brief Starts the server's operation by setting up sockets, epoll, and handling events.
 *
 * @details This method initializes the server's operation, including setting up sockets
 *          on specified ports, configuring epoll for event handling, and continuously
 *          handling epoll events until instructed to stop.
 *
 * @pre The server must have been properly initialized before calling this method.
 * @post After the method execution, the server will have completed its operation and
 *       released any resources acquired during its runtime.
 * @exception ExceptionType A 'ExceptionType' exception may be thrown in case of errors
 *                          during socket setup or epoll configuration.
 *
 * @note This method is public and forms the core functionality of the server.
 */
void Server::start() {
    run = true;
    signal(SIGINT, signal_handler); // Register signal handler
	
	// Initialize sockets and start listening
    if (!initializeSockets())
        return; // Check if id ont forget to clsoe everything!
	
	// Set up epoll
    epoll_fd = setUpEpoll();
	if (epoll_fd == -1) {
	    std::cerr << "Failed to set up epoll" << std::endl;
		cleanup();
        return;
    }
	
    while (run) {
        if (handle_epoll_events(epoll_fd) == -1)
            break;
    }

	cleanup();
}

/**
 * @brief Initializes a socket for a specific port.
 *
 * @param ad A pointer to the addrinfo structure containing socket details.
 * @param sock_listen A pointer to an integer that will store the created socket's descriptor.
 * @param port The port number for which the socket will be initialized.
 *
 * @return 0 on success, -1 on error.
 *
 * @exception ExceptionType A 'ExceptionType' exception may be thrown in case of errors
 *                          during socket initialization, option configuration, or binding.
 * @details This method initializes a socket for a specific port using the provided 'addrinfo'
 *          structure. The socket is created with the specified address family, socket type,
 *          and protocol. It also sets the socket option 'SO_REUSEADDR' to avoid "Address
 *          already in use" errors. The socket is then bound to the provided address and port.
 *          If any step fails, an error code is returned.
 * @post If successful, the 'sock_listen' parameter will be populated with a valid socket
 *       descriptor ready for use.
 * @usage This method is typically called inside a loop that iterates over multiple ports to
 *        prepare sockets for accepting connections on each port. If an error occurs during
 *        the setup of a socket, the entire server might be closed rather than continuing
 *        with non-functional ports.
 * @note This method is used internally and is not intended to be directly called by user code.
 */
bool Server::initializeSockets() {
    std::vector<std::string> ports = getPorts();
    for (std::vector<std::string>::const_iterator it = ports.begin();
		it != ports.end();
		++it) {
			
        int socket;
        if (setUpSocket(&socket, *it) == -1) {
            std::cerr << "Failed to set up socket at port " << *it << std::endl;
			cleanup();
            return false;
        }
		
        sock_listens.push_back(socket);
        if (listen(socket, BACKLOG) == -1) {
            perror("listen");
			cleanup();
            return false; 
        }
        std::cout << "Listening on port: " << *it << std::endl;
    }
    return true;	
}


void Server::cleanup() {
    if (epoll_fd != -1)
		close(epoll_fd);
    for (	std::vector<int>::iterator it = sock_listens.begin();
			it != sock_listens.end();
			++it) {
        close(*it);
    }
}


/**
 * @brief Initializes a socket for a specific port.
 *
 * @param ad A pointer to the addrinfo structure containing socket details.
 * @param sock_listen A pointer to an integer that will store the created socket's descriptor.
 * @param port The port number for which the socket will be initialized.
 *
 * @return 0 on success, -1 on error.
 *
 * @exception ExceptionType A 'ExceptionType' exception may be thrown in case of errors
 *                          during socket initialization, option configuration, or binding.
 * @details This method initializes a socket for a specific port using the provided 'addrinfo'
 *          structure. The socket is created with the specified address family, socket type,
 *          and protocol. It also sets the socket option 'SO_REUSEADDR' to avoid "Address
 *          already in use" errors. The socket is then bound to the provided address and port.
 *          If any step fails, an error code is returned.
 * @post If successful, the 'sock_listen' parameter will be populated with a valid socket
 *       descriptor ready for use.
 * @usage This method is typically called inside a loop that iterates over multiple ports to
 *        prepare sockets for accepting connections on each port. If an error occurs during
 *        the setup of a socket, the entire server might be closed rather than continuing
 *        with non-functional ports.
 */
int Server::initializeSocket(const addrinfo* ad,
							int* sock_listen,
							const std::string& port) {

	int optval = 1;
    // Create a socket with the specified address family, socket type, and protocol				
    *sock_listen = socket(ad->ai_family, ad->ai_socktype, ad->ai_protocol);
    if (*sock_listen == -1) {
        std::cerr << "Socket error: " << strerror(errno) << std::endl;
        return -1;
    }
	std::cout << "Socket created for port: " << port << std::endl;
	
	// Set the socket option to reuse the address (SO_REUSEADDR) to avoid "Address already in use" errors
    if (setsockopt(*sock_listen, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
        std::cerr << "Setsockopt error: " << strerror(errno) << std::endl;
        close(*sock_listen);
        return -1;
    }

    // Bind the socket to the provided address and port
    if (bind(*sock_listen, ad->ai_addr, ad->ai_addrlen) == -1) {
        std::cerr << "Bind error: " << strerror(errno) << std::endl;
        close(*sock_listen);
        return -1;
    }

    std::cout << "Bind successful for port: " << port << std::endl;
    return 0;
}

/**
 * @brief Prepares a socket for accepting connections.
 *
 * @param sock_listen A pointer to an integer that will store the created socket's descriptor.
 * @param port The port number for which the socket will be initialized.
 *
 * @return 0 on success, -1 on error.
 *
 * @exception ExceptionType A 'ExceptionType' exception may be thrown in case of errors
 *                          during socket setup or binding.
 * @details This function sets up a socket for accepting incoming connections. It uses TCP
 *          over IPv4 (AF_INET and SOCK_STREAM) to create the socket. The port is defined by
 *          the provided 'port' parameter. The function iterates over the possible addresses
 *          returned by getaddrinfo, attempting to bind to each one until it succeeds or
 *          exhausts all options. If successful, the initialized socket descriptor is stored
 *          in 'sock_listen'. If binding fails for all addresses, an error code is returned.
 * @pre The server must have been properly initialized before calling this function.
 * @post If successful, the 'sock_listen' parameter will be populated with a valid socket
 *       descriptor that's ready for accepting connections.
 * @note This method is private and forms an essential part of the server's socket setup process.
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
		freeaddrinfo(addrs);
		throw std::runtime_error("Failed to initialize socket");
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
 * @brief Sets up epoll for managing socket events.
 *
 * @return The epoll file descriptor on success, -1 on error.
 *
 * @details This method initializes an epoll instance for efficiently managing socket
 *          events. For each socket descriptor in the 'sock_listens' collection, it sets up
 *          an epoll event structure and registers the socket for read events. The method
 *          then adds each socket to the epoll instance. If successful, the epoll file
 *          descriptor is returned; otherwise, an error code is returned.
 * @pre The server's listening sockets should have been successfully initialized and stored
 *      in the 'sock_listens' collection.
 * @post The epoll instance will be set up to efficiently handle incoming socket events.
 * @usage This method is typically invoked within the server's setup routine, specifically
 *        after initializing the server's listening sockets. The returned epoll file
 *        descriptor is used for efficient event handling during the server's operation.
 * @note The epoll instance is a key component for efficiently managing multiple socket
 *       events. Errors during epoll creation or socket event registration are handled,
 *       and the method returns -1 to indicate failure.
 */
int Server::setUpEpoll() {
	// Create epoll
	int epoll_fd = epoll_create(2);
	if (epoll_fd == -1) {
		perror("epoll_create1");
		return -1;
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
			cleanupEpoll(epoll_fd, it);
			perror(strerror(errno));
			return -1;
		}
	}
	return epoll_fd;
}

/**
 * @brief Cleans up epoll and closes sockets.
 * 
 * @param epoll_fd 
 * @param failed_it 
 * @return true 
 * @return false 
 */
bool Server::cleanupEpoll(int epoll_fd, std::vector<int>::iterator failed_it) {
    for (std::vector<int>::iterator it = failed_it; it != sock_listens.begin(); --it) {
        if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, *it, NULL) == -1) {
            perror("epoll_ctl EPOLL_CTL_DEL");
        }
    }
    return false; 
}


/**
 * @brief Accepts a new client connection and adds it to epoll.
 *
 * @param epoll_fd The epoll file descriptor for managing socket events.
 * @param sock_listen The listening socket descriptor.
 *
 * @return The socket descriptor of the newly connected client on success, -1 on error.
 *
 * @exception ExceptionType A 'ExceptionType' exception may be thrown in case of errors
 *                          during accepting a new connection or adding it to epoll.
 * @details This method accepts a new client connection on the provided 'sock_listen'
 *          socket descriptor. Once a new client is connected, it is added to epoll for
 *          efficient event management. The client's socket descriptor is returned on
 *          success, and -1 is returned on error.
 * @pre The server's listening socket should have been successfully initialized and bound
 *      to a port.
 * @post The newly connected client will be added to epoll for event handling.
 * @usage This method is called within the server's main event loop to accept new client
 *        connections and integrate them into the epoll event management system.
 * @note If the new client connection or the addition to epoll fails, corresponding error
 *       messages are printed, and -1 is returned to indicate failure.
 */
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


int Server::calculate_dynamic_timeout() {
	// Logic to determine appropriate timeout
	int timeout_value = 1000;
	return timeout_value;
}


/**
 * @brief Handles epoll events for efficient socket event management.
 *
 * @param epoll_fd The epoll file descriptor for managing socket events.
 *
 * @return 0 on success, -1 on error.
 *
 * @exception ExceptionType A 'ExceptionType' exception may be thrown in case of errors
 *                          during epoll event handling or processing of sockets.
 * @details This method is responsible for efficiently handling epoll events. It waits
 *          for epoll events using 'epoll_wait' and processes the events that occur in 
 * 			a no-blocking manner.
 *          For each event, it inspects and processes either the listening socket or
 *          a client socket, as appropriate. The method returns 0 on successful event
 *          handling and -1 on error.
 * @pre The epoll instance should have been set up successfully, and the server's sockets
 *      should have been initialized and added to epoll.
 * @post Epoll events will be efficiently handled, and appropriate actions will be taken
 *       based on the type of event (EPOLLIN, EPOLLOUT, EPOLLER).
 * @note The 'sock_listens' collection is used to differentiate between listening sockets
 *       and client sockets. Error handling during epoll event processing or socket
 *       processing is handled by returning -1.
 */

int Server::handle_epoll_events(int epoll_fd) {

    struct epoll_event	events[MAX_EVENTS];
    HttpRequestBase		request;
    int					num_fds;

    int timeout = calculate_dynamic_timeout();

    std::cout << "Waiting for an epoll event..." << std::endl;
    num_fds = epoll_wait(epoll_fd, events, MAX_EVENTS, timeout);
    if (num_fds == -1) {
        return handle_epoll_error();
    }

    std::cout << "Epoll returned with " << num_fds << " events." << std::endl;

    for (int i = 0; i < num_fds; i++) {
        inspect_epoll_event(events[i].events);

        if (std::find(sock_listens.begin(), sock_listens.end(), events[i].data.fd) != sock_listens.end()) {
            process_listen_socket(epoll_fd, events[i]);
        } else {
            process_client_socket(epoll_fd, events[i]);
        }
    }
    return 0;
}

/**
 * @brief Handles errors during epoll event processingand is our way to exit the server safely.
 *
 * @return -1 indicating an error condition.
 *
 * @note This method is a simple helper function for handling epoll-related errors. It
 *       does not involve any complex logic and is used to handle error conditions
 *       consistently.
 */int Server::handle_epoll_error() {
	if (errno == EINTR)
		perror("Safe exit");
	else
		perror("epoll_wait");
	return -1;
}

/**
 * @brief Processes events on a listening socket for new client connections.
 *
 * @param epoll_fd The epoll file descriptor for managing socket events.
 * @param event The epoll event structure for the listening socket.
 *
 * @usage This method is generally called from the 'handle_epoll_events' routine when an
 *        epoll event occurs on a listening socket. It enables the server to efficiently
 *        handle new incoming client connections.
 */
void Server::process_listen_socket(int epoll_fd, struct epoll_event& event) {
    
	if (accept_new_client(epoll_fd, event.data.fd) == -1) {
        if (!(errno == EAGAIN || errno == EWOULDBLOCK)) {
            perror("accept");
        }
    }
}

/**
 * @brief Processes events on a client socket.
 *
 * @param epoll_fd The epoll file descriptor for managing socket events.
 * @param event The epoll event structure for the client socket.
 *
 * @details This method handles events occurring on a client socket. It logs the client's
 *          socket event and attempts to handle the client's event using the 'handleClientEvent'
 *          method. If an exception is thrown during event handling, the error message is logged.
 */
void Server::process_client_socket(int epoll_fd, struct epoll_event& event) {
    
	std::cout << "Handling client " << event.data.fd << " event" << std::endl;
    try {
        handleClientEvent(epoll_fd, event);
    } catch (const std::exception& e) {
        std::cerr << "Error handling client event: " << e.what() << std::endl;
    }
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
		std::cerr << defaultConf + ": error: failed to open default config file." << std::endl;
		exit(EXIT_FAILURE);
	}
	_config = Config();
	if (_config.setupConf(file, defaultConf)) {
		std::cerr << defaultConf + ": error: could not setup default config, aborting." << std::endl;
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
		std::cerr << configPath + ": error: failed to open config file, using default config instead." << std::endl;
		loadDefaultConfig();
		return ;
	}
	_config = Config();
	if (_config.setupConf(file, configPath)) {
		std::cerr << configPath + ": error: could not setup this config, using default config instead." << std::endl;
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


