/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Timeout.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/03/11 01:18:42 by rgarrigo          #+#    #+#             */
/*   Updated: 2023/09/12 16:12:48 by motero           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Config.hpp"

void    Server::addTimeoutEvent(int clientFd, const std::string& typedefName) {
    int timeout;
    
    if (typedefName.empty())
        return ;
    
    //Fetch the timeout value from the config file    
    Config& config = this->_config;
    t_globalConfig globalConfig;
    config.getGlobalConfig(globalConfig);
    
    if (typedefName == "client")
        timeout = globalConfig.timeoutClient;
    else if (typedefName == "cgi")
        timeout = globalConfig.timeoutCgi;
    else
        return ;
    
    //Get current time
    std::time_t currentTime;
    time(&currentTime);
    
    //Create a timeout_event instamnce
    t_timeOutEvent  newEvent;
    newEvent.expirationTime = currentTime + timeout / 1000;
    newEvent.event_fd = clientFd;

    // Add the new event to your priority queue
    _timeOutEvents.push(newEvent);
}

int Server::initializeSelfPipe() {
    int selfPipe[2];
    if (pipe(selfPipe) == -1) {
        log_message(Logger::ERROR, "pipe %d", errno);
        throw std::exception();
        return -1;
    }

    selfPipeReadFd = selfPipe[0];
    selfPipeWriteFd = selfPipe[1];

    // fcntl(selfPipeReadFd, F_SETFL, O_NONBLOCK);
    // fcntl(selfPipeWriteFd, F_SETFL, O_NONBLOCK);

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = selfPipeReadFd;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, selfPipeReadFd, &ev) == -1) {
        log_message(Logger::ERROR, "epoll_ctl %d", errno);
        throw std::exception();
    }
    return 0;
}

void    Server::checkAndHandleTimeouts() {
    
    while (!_timeOutEvents.empty()) {
        // Get the top event from the priority queue
        t_timeOutEvent topEvent = _timeOutEvents.top();

        std::time_t currentTime;
        time(&currentTime);
        
        // If the top event has not expired, break
        if  (topEvent.expirationTime > currentTime) {
            break;
        }
        write(selfPipeWriteFd, "x", 1);
        break ;
    }
}

void    Server::handleTimeoutEvent(int epoll_fd) {
    char    buf;
    read(selfPipeReadFd, &buf, 1);

    std::time_t currentTime;
    time(&currentTime);

    log_message(Logger::WARN, "Handling timeout events");
    log_message(Logger::WARN, "\t\t_timeOutEvents.size() = %d", _timeOutEvents.size());
    while (!_timeOutEvents.empty()) {
        const t_timeOutEvent& topEvent = _timeOutEvents.top();

        // If the top event has not expired, break
        if  (topEvent.expirationTime > currentTime) {
            break;
        }
        struct epoll_event ev;
        memset(&ev, 0, sizeof(ev));
        if (handleEvent(epoll_fd, ev, topEvent.event_fd, true)) {
            close_and_cleanup(epoll_fd, topEvent.event_fd);
        }
        log_message(Logger::DEBUG, "Timeout event for fd %d", topEvent.event_fd);
        //_timeOutEvents.pop();
        log_message(Logger::WARN, "\t\t_timeOutEvents.size() = %d", _timeOutEvents.size());
    }
}

void Server::removeTimeoutEvent(int fdToRemove) {
    std::priority_queue<t_timeOutEvent> newQueue;

    while (!_timeOutEvents.empty()) {
        t_timeOutEvent topEvent = _timeOutEvents.top();
        _timeOutEvents.pop();
        if (topEvent.event_fd != fdToRemove) {
            newQueue.push(topEvent);
        }
    }

    _timeOutEvents = newQueue;
}
