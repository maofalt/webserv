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

#include "ClientHandler.hpp" 

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
#include "HttpRequest.hpp"

class Server {
private:
    int                         epoll_fd;
    std::vector<int>            sock_listens;  // to list to multiple ports
    std::map<int, HttpRequest>  ongoingRequests;  // ongoing requests for each client_fd

    // Configuration details can go here.
    // E.g., struct Config or std::map<std::string, std::string> config;
    
    void    setUpSocket(int port);  // For initializing and binding a socket for a given port
    int     setUpEpoll(int sock_listen);  // Setting up epoll
    int     accept_new_client(int sock_listen);  // Handling new client connection
    void    handle_client_data(int client_fd);  // Handling data from a client
    
public:
    Server();  // Constructor might accept configuration file path as an argument
    Server(const Server& other);
    Server& operator=(const Server& other);
    ~Server();

    void start();  // Start the server
    void stop();  // Stop the server

    void loadConfig(const std::string& configPath);  // Load server configurations from a file

    friend std::ostream& operator<<(std::ostream& os, const Server & server);
};
#endif