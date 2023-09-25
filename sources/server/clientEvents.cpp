/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   clientEvents.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/28 17:29:05 by znogueir          #+#    #+#             */
/*   Updated: 2023/09/25 01:49:36 by rgarrigo         ###   ########.fr       */
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
bool	Server::validateClient(int client_fd) {
	if (clientHandlers.find(client_fd) == clientHandlers.end()) {
		log_message(Logger::DEBUG, "Unknown client fd: %d, IT MUST BE A CGI !", client_fd);
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
int		Server::accept_new_client(int epoll_fd, int sock_listen) {

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

	addTimeoutEvent(sock_server, "client");
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
int		Server::handleFdEvent(int epoll_fd, struct epoll_event& event) {
	int eventFd = event.data.fd;
	
	if (handleEvent(epoll_fd, event, eventFd, false)) {
		return -1;
	}
	return 0;
}

int		Server::handleEvent(int epoll_fd, struct epoll_event& event, int eventFd, bool timeout) {
	int clientFd =  eventFd;
	//if eventFd is a cgiFd, get the clientFd
	if(validateClient(eventFd)) {
		clientFd = _cgiFdsToClientFd[eventFd];	
	}

	//Get the clientHandler corresponding to the fd and requesting a vector of changes to  client handler
	ClientHandler& client = clientHandlers[clientFd];
	std::vector<t_epollSwitch> epollSwitch = client.handleEvent(eventFd, event, timeout);

	//update epoll (ADD, MOD, DEL)
	if (updateEpoll(epoll_fd, clientFd, epollSwitch)) {
		return -1;	
	}
	//update timeout events
	if(updateTimeoutEvents(epollSwitch)) {
		return -1;
	}
	return 0;
}

//method to be cleaned ! separed in two different methods
int Server::updateEpoll(int epoll_fd, int clientFd, std::vector<t_epollSwitch>& epollSwitch) {
    std::vector<t_epollSwitch>::iterator it = epollSwitch.begin();
    
    while (it != epollSwitch.end()) {
        int op = determineOperation(it, clientFd);
        int mode = determineMode(it);

        if (changeClientEpollMode(epoll_fd, it->fd, mode, op)) {
            log_message(Logger::ERROR, "epoll_ctl: problem with during switch");
        }

        if (it->mode == DEL) {
            it = epollSwitch.erase(it);
        } else {
            ++it;
        }
    }
    
    return 0;
}

int Server::determineOperation(std::vector<t_epollSwitch>::iterator it, int clientFd) {
    int op;

    if (trackFds.find(it->fd) != trackFds.end()) {
        op = EPOLL_CTL_MOD;
        if (it->mode == DEL) {
            op = EPOLL_CTL_DEL;
        }
    } else if (it->mode == DEL) {
        struct stat buf;
        if (fstat(it->fd, &buf) == 0) {
            close(it->fd);
        }
		log_message(Logger::WARN, "fd %d is going to be deleted BUT fd i not tracked", it->fd);
        // Skipping the current iteration as the fd is to be deleted
        return -1;
    } else {
        op = EPOLL_CTL_ADD;
        trackFds.insert(it->fd);
        if (it->fd != clientFd) {
            _cgiFdsToClientFd[it->fd] = clientFd;
        }
    }

    return op;
}

int Server::determineMode(std::vector<t_epollSwitch>::iterator it) {
    int mode;
	
	mode = 0;
    switch (it->mode) {
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
			break;
        default:
            mode = 0;
            break;
    }

    return mode;
}


int Server::updateTimeoutEvents(std::vector<t_epollSwitch>& epollSwitch) {
    std::priority_queue<t_timeOutEvent> newTimeoutEvents;
    std::map<int, std::time_t> timeoutUpdates;

    createTimeoutUpdatesMap(epollSwitch, timeoutUpdates);
    updateExistingTimeoutEvents(newTimeoutEvents, timeoutUpdates);
    addNewTimeoutEvents(newTimeoutEvents, timeoutUpdates);
    
    _timeOutEvents = newTimeoutEvents;
    
    return 0;
}

void Server::createTimeoutUpdatesMap(const std::vector<t_epollSwitch>& epollSwitch, 
                                     std::map<int, std::time_t>& timeoutUpdates) {
    for (size_t i = 0; i < epollSwitch.size(); ++i) {
        timeoutUpdates[epollSwitch[i].fd] = epollSwitch[i].timeout;
    }
}

void Server::updateExistingTimeoutEvents(std::priority_queue<t_timeOutEvent>& newTimeoutEvents, 
                                         std::map<int, std::time_t>& timeoutUpdates) {
    while (!_timeOutEvents.empty() && !timeoutUpdates.empty()) {
        t_timeOutEvent topEvent = _timeOutEvents.top();
        _timeOutEvents.pop();
        std::map<int, std::time_t>::iterator it = timeoutUpdates.find(topEvent.event_fd);

        if (it != timeoutUpdates.end()) {
            topEvent.expirationTime = it->second;
            timeoutUpdates.erase(it);
        }

        newTimeoutEvents.push(topEvent);
    }
}

void Server::addNewTimeoutEvents(std::priority_queue<t_timeOutEvent>& newTimeoutEvents, 
                                 const std::map<int, std::time_t>& timeoutUpdates) {
    for (std::map<int, std::time_t>::const_iterator it = timeoutUpdates.begin(); it != timeoutUpdates.end(); ++it) {
        t_timeOutEvent newEvent;
        newEvent.event_fd = it->first;
        newEvent.expirationTime = it->second;

        newTimeoutEvents.push(newEvent);
    }
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
int		Server::changeClientEpollMode(int epoll_fd, int client_fd, int mode) {

	struct epoll_event ev;
	ev.events = mode | EPOLLET;
	ev.data.fd = client_fd;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_fd, &ev) == -1) {
		log_message(Logger::ERROR, "epoll_ctl: EPOLL_CTL_MOD");
		return -1;
	}
	return 0;
}

int		Server::changeClientEpollMode(int epoll_fd, int client_fd, u_int32_t mode, int op) {

	struct epoll_event ev;
	if (mode) {
		ev.events = mode | EPOLLET;
		ev.data.fd = client_fd;
		if (epoll_ctl(epoll_fd, op, client_fd, &ev) == -1) {
		log_message(Logger::ERROR, "epoll_ctl: problem with during switch");
		return -1;
		}
	} else
		log_message(Logger::DEBUG, "mode is 0, nothing to do");

	if (op == EPOLL_CTL_DEL) {
		trackFds.erase(client_fd);
		_cgiFdsToClientFd.erase(client_fd);
		removeTimeoutEvent(client_fd);
		clientHandlers.erase(client_fd);
		
		//check with stat fi fd is openif yes closeis
		log_message(Logger::DEBUG, "fd %d is going to be deleted", client_fd);
		struct stat buf;
		if (fstat(client_fd, &buf) == 0)
			close(client_fd);
	}
	return 0;
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
void	Server::handleEpollError(int client_fd) {
	std::cerr << "Error on client fd: " << client_fd << std::endl;
	throw std::runtime_error("EPOLL error occurred on client fd");
}
