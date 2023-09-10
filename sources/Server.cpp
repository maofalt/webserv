/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/03/11 01:18:42 by rgarrigo          #+#    #+#             */
/*   Updated: 2023/09/07 18:17:33 by motero           ###   ########.fr       */
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
//	Logger::cleanup();
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
	log_message(Logger::INFO, "Initializing sockets");
	std::set<std::string> ports = getPorts();
	log_message(Logger::INFO, "Ports to listen: %lu", ports.size());
	for (std::set<std::string>::const_iterator it = ports.begin();
			it != ports.end();
			++it) {

		int socket;
		if (setUpSocket(&socket, *it) == -1) {
			std::cerr << "Failed to set up socket at port " << *it << std::endl;
			cleanup();
			return false;
		}

		sock_listens.push_back(socket);
		if (listen(socket, SOMAXCONN) == -1) {
			perror("listen");
			cleanup();
			return false; 
		}
		log_message(Logger::INFO, "Listening on port: %s", it->c_str());
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
	for(std::map<int, int>::iterator it2 = _cgiFdsToClientFd.begin(); it2 != _cgiFdsToClientFd.end(); ++it2) {
		close(it2->first);
	}
	for (std::set<int>::iterator it = trackFds.begin(); it != trackFds.end(); ++it) {
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
	log_message(Logger::INFO, "Socket created for port: %s", port.c_str());

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
	
	ClientHandler::addPort(*sock_listen, ntohs(((struct sockaddr_in *)ad->ai_addr)->sin_port));

	log_message(Logger::INFO, "Bind successful for port: %s", port.c_str());
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
	hints.ai_family = AF_INET;
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

int Server::calculate_dynamic_timeout() {
	// Logic to determine appropriate timeout
	int timeout_value = 10000;
	return timeout_value;
}

/**
 * @brief Signal handler for handling specific signals.
 *
 * @param sig The signal number that triggered the handler.
 *
 * @note Proper signal handling is important for gracefully shutting down a server when
 *       specific signals are received.
 */
void	Server::signal_handler(int sig)
{
	if (sig == SIGINT) {
		Server::run = false;
	}
}

//This method will probably dissapear or change completely, just here to bootstrap 
// multiple ports
std::set<std::string> Server::getPorts() {
//Fetch all server configuration
	Config& 					config		=	this->getConfig();
	std::vector<ServerConfig>& 	servList	=	config.getServList();
	
//fetch all ports inside each server	
	std::set<std::string>		ports;
	for (std::vector<ServerConfig>::iterator it = servList.begin() ;
		it != servList.end(); ++it) {
		
		std::set<std::string> tmp = it->getPorts();
		ports.insert(tmp.begin(), tmp.end());
	}
	
	return ports;
}

/**
 * @brief Stops the server's operation gracefully.
 *
 * @details This method is responsible for stopping the server's operation in a graceful
 *          manner. It sets the 'run' flag to false, indicating that the server should
 *          stop processing new requests. Additionally, it closes all the listening sockets
 *          and the epoll file descriptor, if they were previously opened.
 */
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

	// Cleanup Logger
    Logger::cleanup();
}

/**
 * @brief Loads the default configuration settings for the server.
 *
 * @details This method is responsible for loading the default configuration settings
 *          for the server from a configuration file. It reads the default configuration
 *          file and sets up the `_config` object with the loaded settings. 
 */
int Server::loadDefaultConfig() {
	std::ifstream	file;

	file.open(defaultConf.c_str(), std::fstream::in);
	if (!file) {
		std::cerr << BOLD << defaultConf + ": "<< RED << "error: " << RESET;
		std::cerr << "failed to open default config file." << std::endl;
		return 1;
	}
	_config = Config();
	if (_config.setupConf(file, defaultConf)) {
		std::cerr << BOLD << defaultConf << RESET;
		std::cerr << ": could not setup default config, aborting." << std::endl;
		return 1;
	}
	ClientHandler::setConfig(_config);
	INFO_LOG("Config = " + defaultConf);
	return 0;
}

/**
 * @brief Loads the server configuration settings from the given path.
 *
 * @details This method is responsible for loading the server configuration settings
 *          from the specified configuration file path. It reads the configuration file
 *          and sets up the `_config` object with the loaded settings. If the specified
 *          configuration file cannot be opened or an error occurs during setup, the
 *          method falls back to loading the default configuration settings.
 * @param configPath The path to the configuration file to load.
 * @usage Call this method to load server configuration settings from the specified path
 * 		  at start.
 * @note This method offers the flexibility to load configuration settings from a
 *       user-specified path, and if that fails, it will revert to loading the default
 *       configuration settings.
 */
int Server::loadConfig(const std::string& configPath) {
	// Load server configuration from the given path.
	// create a specific object with a new class

	std::ifstream	file;

	file.open(configPath.c_str(), std::fstream::in);
	if (!file) {
		std::cerr << BOLD << configPath + ": "<< RED << "error: " << RESET;
		std::cerr << "failed to open config file, using default config instead." << std::endl;
		return loadDefaultConfig();
	}
	_config = Config();
	if (_config.setupConf(file, configPath)) {
		std::cerr << BOLD << configPath << RESET;
		std::cerr << ": could not setup this config, using default config instead." << std::endl;
		return loadDefaultConfig();
	}
	ClientHandler::setConfig(_config);
	INFO_LOG("Config = " + configPath);

	return 0;
}

bool	Server::loadValidationFile(const std::string& validationPath) {
	
	DEBUG_LOG("Loading validation file");
	_validationFile.loadConfig(validationPath);
	DEBUG_LOG("Validation file loaded");
	_validationFile.printAll();

	return true;
}


std::ostream& operator<<(std::ostream& os, const Server & server);
