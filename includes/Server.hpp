#ifndef SERVER_HPP
#define SERVER_HPP

#include <vector>
#include <map>
#include <string>
#include <cstring>
#include <iostream>
#include <errno.h>  // For error handling
#include <sys/types.h>  // Required for sockets
#include <sys/socket.h>
#include <netdb.h>  // getaddrinfo
#include <fcntl.h>  // O_NONBLOCK
#include <unistd.h>  // close
#include <sys/epoll.h>  // epoll
#include <signal.h>  // signal handling
#include <stdlib.h>
#include <stdio.h>

#include "ClientHandler.hpp" 

#define PORT "8694"
#define PORTAL "8000"
#define BACKLOG 5
#define BUFFER_SIZE 2048
/*
Server Class:

Purpose:    This class should be responsible for setting up 
            and managing the web server.
Attributes:
            - List of ports it listens on.
            - Configuration file details, if any.
            - A pool or collection of ClientHandler objects (each potentially 
            handling a different client).
Methods:
            - Start/Stop server.
            - Accept incoming connections.
            - Dispatch to a ClientHandler.
            - Load and interpret the configuration file.
*/

#include "ClientHandler.hpp"  // Assuming you'll create this
#include "HttpRequestBase.hpp"

class Server {
private:
    int                             epoll_fd;
    std::vector<int>                sock_listens;  // to list to multiple ports
    std::map<int, HttpRequestBase>  ongoingRequests;  // ongoing requests for each client_fd
    std::map<int, ClientHandler>    clientHandlers;  // ongoing requests for each client_fd

    static volatile sig_atomic_t	    run;
    // Configuration details can go here.
    // E.g., struct Config or std::map<std::string, std::string> config;
    
public:
    Server();  // Constructor might accept configuration file path as an argument
    ~Server();
private://This way they can't be used, since it doesn't make sense implementing them
    Server(const Server& other);
    Server& operator=(const Server& other);
    
public:
    void    loadConfig(const std::string& configPath);  // Load server configurations from a file
    void    start();  // Start the server
    void    stop();  // Stop the server

private:
    // Encapsulate all the helper methods inside private scope
    std::vector<std::string>    getPorts();
    int                         setUpSocket(int* sock_listen, const std::string& port);
    int                         calculate_dynamic_timeout();
    static void	                signal_handler(int sig);

    //handle cllient Methods
    int                         accept_new_client(int epoll_fd, int sock_listen);
    int                         changeClientEpollMode(int epoll_fd, int client_fd, int mode);
    int                         handleClientEvent(int epoll_fd, struct epoll_event& event);
    void                        handle_client_data(int epoll_fd, int client_fd);
    
    //Multiplexing methods
    int                         setUpEpoll();
    void                        inspect_epoll_event(uint32_t events);
    int                         handle_epoll_events(int epoll_fd);
    void                        close_and_cleanup(int epoll_fd, int client_fd);

public:
    friend std::ostream& operator<<(std::ostream& os, const Server & server);
};


#endif