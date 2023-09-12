/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   clientEvents.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: znogueir <znogueir@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/28 17:29:05 by znogueir          #+#    #+#             */
/*   Updated: 2023/09/11 16:12:15 by znogueir         ###   ########.fr       */
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
bool Server::validateClient(int client_fd) {
	if (clientHandlers.find(client_fd) == clientHandlers.end()) {
		std::cerr << "Unknown client fd: " << client_fd << std::endl;
		return 1;
	}
	return 0;
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

	//fcntl(sock_server, F_SETFL, O_NONBLOCK);
	log_message(Logger::DEBUG, "New client connected on fd: %d", sock_server);

	//add to Epoll
	event.events = EPOLLIN | EPOLLET;
	event.data.fd = sock_server;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock_server, &event) == -1) {
		log_message(Logger::ERROR, "epoll_ctl: EPOLL_CTL_ADD");
		close(sock_server);
		return -1;
	}
	
	log_message(Logger::DEBUG, "Added new client on fd: %d to epoll", sock_server);
	//add to clientHandlers
	clientHandlers[sock_server] = ClientHandler(sock_listen, sock_server);
	trackFds.insert(sock_server);
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
int Server::handleFdEvent(int epoll_fd, struct epoll_event& event) {
	int eventFd = event.data.fd;
	int clientFd =  eventFd;

	if(validateClient(eventFd)) {
		log_message(Logger::ERROR, "%d fd is not a client but cgi", eventFd);
		clientFd = _cgiFdsToClientFd[eventFd];	
	}

	ClientHandler& client = clientHandlers[clientFd];
	//log_message(Logger::INFO, "Handling event on event fd: %d", eventFd);
	//inspect_epoll_event(event.events);

	// Depending on the epoll event, decide the action on the client

//	if (event.events & EPOLLHUP) { 
//		log_message(Logger::ERROR, "EPOLLHUP for fd %d", eventFd);
//		close_and_cleanup(epoll_fd, clientFd);
//		return -1;
//	}
	// if (event.events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
	// 	log_message(Logger::ERROR, "EPOLLERR | EPOLLHUP | EPOLLRDHUP for fd %d", eventFd);
	// 	handleEpollError(eventFd);
	// }

	std::vector<t_epollSwitch> epollSwitch = client.handleEvent(eventFd, event, false);

	if (updateEpoll(epoll_fd, clientFd, epollSwitch)) {
		
		return -1;	
	}
	return 0;
}

//method to be cleaned ! separed in two different methods
int	Server::updateEpoll(int epoll_fd, int clientFd, std::vector<t_epollSwitch>& epollSwitch) {

	int op;
	int mode;
	
	for (std::vector<t_epollSwitch>::iterator it = epollSwitch.begin(); 
		it != epollSwitch.end(); ++it ) {
		//Set op depending on if fd is already in trackFds

		log_message(Logger::DEBUG, "fd %d is going to be %s", it->fd, it->mode == DEL ? "deleted" : "added");
		if (trackFds.find(it->fd) != trackFds.end()) {
			op = EPOLL_CTL_MOD;
			if (it->mode ==  DEL)
				op = EPOLL_CTL_DEL;
		} else if (it->mode == DEL){
			log_message(Logger::WARN, "fd %d is going to be deleted", it->fd);
			struct stat buf;
			if (fstat(it->fd, &buf) == 0)
				close(it->fd);
			continue;
		} else {
			op = EPOLL_CTL_ADD;
			trackFds.insert(it->fd);
			if (it->fd != clientFd)
				_cgiFdsToClientFd[it->fd] = clientFd;
		}
		// create mode depending ine_epollMode struct received
		switch (it->mode)
		{
		case IN:
			mode = EPOLLIN;
			break;
		case OUT:
			mode = EPOLLOUT;
			break;
		case IN_OUT:
			mode = EPOLLIN | EPOLLOUT;
			break;
		case DEL:
			mode = 0;
			break;
		default:
			mode = 0;
			break;
		}
		if (changeClientEpollMode(epoll_fd, it->fd, mode, op)) {
			log_message(Logger::ERROR, "epoll_ctl: problem with during switch");
		}
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
	ev.events = mode | EPOLLET;
	ev.data.fd = client_fd;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_fd, &ev) == -1) {
		log_message(Logger::ERROR, "epoll_ctl: EPOLL_CTL_MOD");
		return -1;
	}
	return 0;
}

int	Server::changeClientEpollMode(int epoll_fd, int client_fd, u_int32_t mode, int op) {

	struct epoll_event ev;
	if (mode) {
		ev.events = mode | EPOLLET;
		ev.data.fd = client_fd;
	}
	if (epoll_ctl(epoll_fd, op, client_fd, &ev) == -1) {
		log_message(Logger::ERROR, "epoll_ctl: problem with during switch");
		return -1;
	}
	if (op == EPOLL_CTL_DEL) {
		trackFds.erase(client_fd);
		_cgiFdsToClientFd.erase(client_fd);
		//check with stat fi fd is openif yes closeis
		log_message(Logger::DEBUG, "fd %d is going to be deleted", client_fd);
		struct stat buf;
		if (fstat(client_fd, &buf) == 0)
			close(client_fd);
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
// void Server::handleReadEvent(int epoll_fd, ClientHandler& client) {
// 	try {
// 		log_message(Logger::DEBUG, "Reading data from client %d", client.getClientFd());
// 		client.readData();
// 		log_message(Logger::DEBUG, "Data read from client %d", client.getClientFd());
// 	} catch (const std::exception& e) {
// 		throw std::runtime_error("Error reading data from client");
// 	}

// 	if (client.isRequestComplete()) {
// 		handleCompleteRequest(epoll_fd, client);
// 		return;
// 	}
// 	log_message(Logger::WARN, "Request not complete for client %d", client.getClientFd());
// 	//DO NOT REMVOE THIS LINE PLEASE !! BLACK MAGIC
// 	changeClientEpollMode(epoll_fd, client.getClientFd(), EPOLLIN);
// }

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
// void Server::handleCompleteRequest(int epoll_fd, ClientHandler& client) {
// 	log_message(Logger::INFO, "Request complete for client %d", client.getClientFd());
// 	if (changeClientEpollMode(epoll_fd, client.getClientFd() , EPOLLOUT) != 0) {
// 		throw std::runtime_error("Failed to change client epoll mode to EPOLLOUT");
// 	}
// 	log_message(Logger::DEBUG, "Changed epoll mode to EPOLLOUT for client %d", client.getClientFd());
// }

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
// void Server::handleWriteEvent(int epoll_fd, ClientHandler& client, int client_fd) {
// 	try {
// 		log_message(Logger::DEBUG, "Writing response to client %d", client_fd);
// 		client.writeResponse();
// 		log_message(Logger::DEBUG, "Response written to client %d", client_fd);
// 		//maha for keep alive
// 		//changeClientEpollMode(epoll_fd, client_fd, EPOLLIN);
// 		close_and_cleanup(epoll_fd, client_fd);
// 		clientHandlers.erase(client_fd);
// 	} catch (const std::exception& e) {
// 		throw std::runtime_error("Error writing response to client");
// 	}
// }

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
