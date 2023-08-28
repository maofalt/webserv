/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   clientEvents.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: znogueir <znogueir@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/28 17:29:05 by znogueir          #+#    #+#             */
/*   Updated: 2023/08/28 17:33:35 by znogueir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

/**
 * @brief Validates a client socket descriptor.
 *
 * @param client_fd The file descriptor of the client socket to be validated.
 *
 * @details This method verifies the validity of a client socket descriptor by checking
 *          whether it exists in the 'clientHandlers' collection. If the client socket
 *          descriptor is not found, an error message is logged, and a 'std::runtime_error'
 *          exception is thrown, indicating the presence of an unknown client socket descriptor.
 */
void Server::validateClient(int client_fd) {
	if (clientHandlers.find(client_fd) == clientHandlers.end()) {
		std::cerr << "Unknown client fd: " << client_fd << std::endl;
		throw std::runtime_error("Unknown client fd encountered");
	}
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
	clientHandlers[sock_server] = ClientHandler(ntohs(((struct sockaddr_in *)&client_addr)->sin_port), sock_server);
	return sock_server;
}

/**
 * @brief Handles events on a client socket.
 *
 * @param epoll_fd The epoll file descriptor for managing socket events.
 * @param event The epoll event structure for the client socket.
 *
 * @details This method processes events occurring on a client socket based on the epoll
 *          event flags provided in 'event'. It validates the client, inspects the epoll
 *          event, and then directs the appropriate handling procedure based on the event.
 * @return Returns 0 on successful event handling.
 */
int Server::handleClientEvent(int epoll_fd, struct epoll_event& event) {
	int client_fd = event.data.fd;

	validateClient(client_fd);

	ClientHandler& client = clientHandlers[client_fd];
	std::cout << "Client " << client_fd << "exists!!" << std::endl;

	inspect_epoll_event(event.events);

	// Depending on the epoll event, decide the action on the client
	if (event.events & EPOLLIN) {
		handleReadEvent(epoll_fd, client);
	} else if (event.events & EPOLLOUT) {
		handleWriteEvent(epoll_fd, client, client_fd);
	} else if (event.events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
		handleEpollError(client_fd);
	}

	return 0;
}

/**
 * @brief Changes the epoll mode of a client socket.
 *
 * @param epoll_fd The file descriptor of the epoll instance.
 * @param client_fd The file descriptor of the client socket.
 * @param mode The new epoll mode to set for the client socket.
 *             This should be a combination of epoll event flags.
 *
 * @return Returns 0 on success, or -1 on failure.
 *
 * @details This method is responsible for changing the epoll mode of a client socket.
 *          The new epoll mode is specified using the 'mode' parameter, which should be
 *          a combination of epoll event flags such as EPOLLIN, EPOLLOUT, etc. The method
 *          uses the 'epoll_ctl' function to modify the epoll event associated with the
 *          client socket to the desired mode.
 */
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

/**
 * @brief Handles read events on a client socket.
 *
 * @param epoll_fd The file descriptor of the epoll instance.
 * @param client A reference to the ClientHandler associated with the client socket.
 *
 * @details This method is responsible for handling read events on a client socket. It reads data
 *          from the client using the associated 'ClientHandler' and checks if the client's request
 *          is complete. If the request is complete, the method delegates the handling of the complete
 *          request to the 'handleCompleteRequest' method. If there's an error while reading data from
 *          the client, a 'std::runtime_error' exception is thrown.
 */
void Server::handleReadEvent(int epoll_fd, ClientHandler& client) {
	try {
		std::cout << "Reading data" << std::endl;
		client.readData();
		std::cout << "Data read" << std::endl;
	} catch (const std::exception& e) {
		throw std::runtime_error("Error reading data from client");
	}

	if (client.isRequestComplete()) {
		handleCompleteRequest(epoll_fd, client);
	}
}

/**
 * @brief Handles a completed client request.
 *
 * @param epoll_fd The file descriptor of the epoll instance.
 * @param client A reference to the ClientHandler associated with the client socket.
 *
 * @details This method is responsible for handling a completed client request. It is called when
 *          the client's request is fully received and parsed. The method changes the epoll mode of
 *          the client socket to EPOLLOUT to prepare for sending a response back to the client. If
 *          changing the epoll mode fails, a 'std::runtime_error' exception is thrown.
 */
void Server::handleCompleteRequest(int epoll_fd, ClientHandler& client) {
	std::cout << "Request complete" << std::endl;
	if (changeClientEpollMode(epoll_fd, client.getClientFd() , EPOLLOUT) != 0) {
		throw std::runtime_error("Failed to change client epoll mode to EPOLLOUT");
	}
	std::cout << "Changed epoll mode to EPOLLOUT" << std::endl;
}

/**
 * @brief Handles writing a response to the client.
 *
 * @param epoll_fd The file descriptor of the epoll instance.
 * @param client A reference to the ClientHandler associated with the client socket.
 * @param client_fd The file descriptor of the client socket.
 *
 * @details This method is responsible for handling the writing of a response to the client.
 *          It is called when the server is ready to send a response back to the client.
 *          The method invokes the 'writeResponse' method of the 'ClientHandler' to write
 *          the response data to the client socket. After writing the response, it closes
 *          the client socket and performs cleanup operations associated with the client.
 *          If an error occurs during writing or cleanup, a 'std::runtime_error' exception is thrown.
 */
void Server::handleWriteEvent(int epoll_fd, ClientHandler& client, int client_fd) {
	try {
		std::cout << "Writing response" << std::endl;
		client.writeResponse();
		std::cout << "Response written" << std::endl;
		close_and_cleanup(epoll_fd, client_fd);
		clientHandlers.erase(client_fd);
	} catch (const std::exception& e) {
		throw std::runtime_error("Error writing response to client");
	}
}

/**
 * @brief Handles an EPOLL error event on a client socket.
 *
 * @param client_fd The file descriptor of the client socket.
 *
 * @details This method is responsible for handling EPOLL error events that occur on a client socket.
 *          When an EPOLL error, EPOLL hang-up, or similar event is detected on a client socket,
 *          this method is invoked to handle the error. It logs an error message and throws a
 *          'std::runtime_error' exception to indicate that an EPOLL error has occurred on the client socket.
 */
void Server::handleEpollError(int client_fd) {
	std::cerr << "Error on client fd: " << client_fd << std::endl;
	throw std::runtime_error("EPOLL error occurred on client fd");
}