/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   timeout.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/03/11 01:18:42 by rgarrigo          #+#    #+#             */
/*   Updated: 2023/09/13 20:52:31 by motero           ###   ########.fr       */
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
    newEvent.expirationTime = currentTime + timeout;
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
    
    log_message(Logger::DEBUG, "Checking timeouts");
    while (!_timeOutEvents.empty()) {
        // Get the top event from the priority queue
        t_timeOutEvent topEvent = _timeOutEvents.top();

        std::time_t currentTime;
        time(&currentTime);
        
        // If the top event has not expired, break
        if  (topEvent.expirationTime > currentTime) {
            log_message(Logger::DEBUG, "Top event has not expired");
            break;
        }
        log_message(Logger::WARN, "Top event has expired");
        log_message(Logger::WARN, "Top event fd: %d", topEvent.event_fd);
        write(selfPipeWriteFd, "x", 1);
        break ;
    }
}

void    Server::handleTimeoutEvent(int epoll_fd) {
    char    buf;
    read(selfPipeReadFd, &buf, 1);

    log_message(Logger::DEBUG, "Handling timeout event");
    std::time_t currentTime;
    time(&currentTime);
    while (!_timeOutEvents.empty()) {
        log_message(Logger::WARN, "timeoutEvents size %d after kind of pop MichaelJackson", _timeOutEvents.size());
        const t_timeOutEvent& topEvent = _timeOutEvents.top();

        // If the top event has not expired, break
        if  (topEvent.expirationTime > currentTime) {
            break;
        }
        struct epoll_event ev;
        memset(&ev, 0, sizeof(ev));
        log_message(Logger::WARN, "Timeout event fd: %d", topEvent.event_fd);
        log_message(Logger::WARN, "handleEvent from handelTiemoutEvent");
        if (handleEvent(epoll_fd, ev, topEvent.event_fd, true)) {
            close_and_cleanup(epoll_fd, topEvent.event_fd);
        }
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
