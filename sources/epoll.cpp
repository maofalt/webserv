/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   epoll.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: znogueir <znogueir@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/28 17:22:17 by znogueir          #+#    #+#             */
/*   Updated: 2023/08/28 17:35:32 by znogueir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

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
	HttpRequest		request;
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
		//erase the clent from the epoll_fd and the clientHandlers
		close_and_cleanup(epoll_fd, event.data.fd);
		std::cerr << "Error handling client event: " << e.what() << std::endl;
	}
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

void Server::inspect_epoll_event(uint32_t events) {
	// Check and print the type of event
	if (events & EPOLLIN) std::cout << "EPOLLIN Event" << std::endl;
	if (events & EPOLLOUT) std::cout << "EPOLLOUT Event" << std::endl;
	if (events & EPOLLERR) std::cout << "EPOLLERR Event" << std::endl;
	if (events & EPOLLHUP) std::cout << "EPOLLHUP Event" << std::endl;

}

/**
 * @brief Closes the client socket and performs cleanup for the given client.
 *
 * @details This method is responsible for performing necessary cleanup operations
 *          for a client socket that is being closed. It removes the client socket
 *          from the epoll set and then closes the socket. If an error occurs during
 *          the epoll control operation, an error message is printed.
 * @param epoll_fd The file descriptor of the epoll instance.
 * @param client_fd The file descriptor of the client socket to close.
 */
void Server::close_and_cleanup(int epoll_fd, int client_fd) {
	struct epoll_event ev;
	if(epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, &ev) == -1) {
		perror("epoll_ctl: EPOLL_CTL_DEL");
		// Handle error
	}
	close(client_fd);
}