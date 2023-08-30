#ifndef SERVER_HPP
#define SERVER_HPP

#include <map>
#include <vector>
#include <string>
#include <stdio.h>
#include <netdb.h>  // getaddrinfo
#include <fcntl.h>  // O_NONBLOCK
#include <errno.h>  // For error handling
#include <cstring>
#include <signal.h>  // signal handling
#include <stdlib.h>
#include <iostream>
#include <unistd.h>  // close
#include <sys/epoll.h>  // epoll
#include <sys/types.h>  // Required for sockets
#include <sys/socket.h>
#include <stdlib.h>

#include "Config.hpp"
#include "ClientHandler.hpp"
#include "HttpRequest.hpp"
#include "Logger.hpp"
#include "Utils.hpp"
#include "IniParser.hpp"


#define PORT "8694"
#define PORTAL "8000"
#define BACKLOG 5
#define BUFFER_SIZE 2048
#define MAX_EVENTS 10
#define PATH_INI "config/validation/contextFields.ini"
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

class Server {
private:
    int                             epoll_fd;
    std::vector<int>                sock_listens;  // to list to multiple ports
    std::map<int, HttpRequest>      ongoingRequests;  // ongoing requests for each client_fd
    Config                          _config;
    IniParser                       _validationFile;
    std::map<int, ClientHandler>    clientHandlers;  // ongoing requests for each client_fd

    static volatile sig_atomic_t	run;
    std::string                     defaultConf;
    // Configuration details can go here.
    // E.g., struct Config or std::map<std::string, std::string> config;
    
public:
    Server();  // Constructor might accept configuration file path as an argument
    ~Server();
private://This way they can't be used, since it doesn't make sense implementing them
    Server(const Server& other);
    Server& operator=(const Server& other);
    
public:
    int     loadConfig(const std::string& configPath);  // Load server configurations from a file;
    int     loadDefaultConfig(); // Load default configuration from config/default.conf;
    void    start();  // Start the server
    void    stop();  // Stop the server
    Config  getConfig() {return this->_config;};
    bool    loadValidationFile(const std::string& validationPath);

private:
    std::vector<std::string>    getPorts();
    bool                        initializeSockets();
    int                         initializeSocket(const addrinfo* ad, int* sock_listen, const std::string& port);
    int                         setUpSocket(int* sock_listen, const std::string& port);
    int                         calculate_dynamic_timeout();
    static void	                signal_handler(int sig);
    void                        cleanup();

    //handle client Methods
    int                         accept_new_client(int epoll_fd, int sock_listen);
    int                         changeClientEpollMode(int epoll_fd, int client_fd, int mode);
    int                         handleClientEvent(int epoll_fd, struct epoll_event& event);
    void                        validateClient(int client_fd);
    void                        handleReadEvent(int epoll_fd, ClientHandler& client);
    void                        handleCompleteRequest(int epoll_fd, ClientHandler& client);
    void                        handleWriteEvent(int epoll_fd, ClientHandler& client, int client_fd);
    void                        handleEpollError(int client_fd);
    
    //Multiplexing methods
    int                         setUpEpoll();
    void                        inspect_epoll_event(uint32_t events);
    int                         handle_epoll_events(int epoll_fd);
    int                         handle_epoll_error();
    void                        process_listen_socket(int epoll_fd, struct epoll_event& event);
    void                        process_client_socket(int epoll_fd, struct epoll_event& event);
    bool                        cleanupEpoll(int epoll_fd, std::vector<int>::iterator failed_it);
    void                        close_and_cleanup(int epoll_fd, int client_fd);





public:
    friend std::ostream& operator<<(std::ostream& os, const Server & server);
};


#endif
