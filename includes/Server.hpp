#ifndef SERVER_HPP
#define SERVER_HPP

#include <map>
#include <vector>
#include <queue>
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
#define TIMEOUT 10

typedef struct s_timeOutEvent {
    std::time_t expirationTime;
    int         event_fd;

    bool operator<(const s_timeOutEvent& other) const {
        return expirationTime > other.expirationTime;
    }
} t_timeOutEvent;


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
    int                                         epoll_fd;
    int                                         selfPipeReadFd;
    int                                         selfPipeWriteFd;
    Config                                      _config;
    IniParser                                   _validationFile;
    std::vector<int>                            sock_listens;  // to list to multiple ports
    std::map<int, HttpRequest>                  ongoingRequests;  // ongoing requests for each client_fd
    std::map<int, ClientHandler>                clientHandlers;  // ongoing requests for each client_fd
    std::set<int>                               trackFds;  // to track fds for cleanup 
    std::priority_queue<t_timeOutEvent>         _timeOutEvents;  // to track fds for cleanup 
    std::map<int, int>                          _cgiFdsToClientFd;  // to track fds for cleanup     

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
    int                         loadConfig(const std::string& configPath);  // Load server configurations from a file;
    int                         loadDefaultConfig(); // Load default configuration from config/default.conf;
    void                        start();  // Start the server
    void                        stop();  // Stop the server
    Config&                     getConfig() {return this->_config;};
    bool                        loadValidationFile(const std::string& validationPath);
    IniParser&                  getValidationFile() {return this->_validationFile;};

        //All gf getters
    const int&                        getEpollFd();
    const int&                        getSelfPipeReadFd();
    const int&                        getSelfPipeWriteFd();
    const std::vector<int>&           getSockListens();
    const std::set<int>&              getTrackFds();

private:
    std::set<std::string>       getPorts();
    bool                        initializeSockets();
    int                         initializeSocket(const addrinfo* ad, int* sock_listen, const std::string& port);
    int                         setUpSocket(int* sock_listen, const std::string& port);
    int                         calculate_dynamic_timeout();
    static void	                signal_handler(int sig);
    void                        cleanup();

    //handle client Methods
    int                         accept_new_client(int epoll_fd, int sock_listen);
    int                         changeClientEpollMode(int epoll_fd, int client_fd, int mode);
    int                         changeClientEpollMode(int epoll_fd, int client_fd, u_int32_t mode, int op);
    int                         handleFdEvent(int epoll_fd, struct epoll_event& event);
    int	                        handleEvent(int epoll_fd, struct epoll_event& event, int eventFd, bool timeout);
    bool                        validateClient(int client_fd);
    void                        handleReadEvent(int epoll_fd, ClientHandler& client);
    void                        handleTimeoutEvent(int epoll_fd);
    void                        handleCompleteRequest(int epoll_fd, ClientHandler& client);
    void                        handleWriteEvent(int epoll_fd, ClientHandler& client, int client_fd);
    void                        handleEpollError(int client_fd);

    //upaate epoll methods thorugh epollSwitch
    int                         updateEpoll(int epoll_fd, int clientFd, std::vector<t_epollSwitch>& epollSwitch);
    int                         determineOperation(std::vector<t_epollSwitch>::iterator it, int clientFd);
    int                         determineMode(std::vector<t_epollSwitch>::iterator it) ;
    
    //Multiplexing methods
    int                         setUpEpoll();
    void                        inspect_epoll_event(uint32_t events);
    int                         handle_epoll_events(int epoll_fd);
    int                         handle_epoll_error();
    void                        process_listen_socket(int epoll_fd, struct epoll_event& event);
    void                        processEvent(int epoll_fd, struct epoll_event& event);
    bool                        cleanupEpoll(int epoll_fd, std::vector<int>::iterator failed_it);
    bool                        cleanupClientPackage(int epoll_fd, std::vector<t_epollSwitch>& epollSwitch);
    void                        close_and_cleanup(int epoll_fd, int client_fd);

    //Timeout methods
    void                        addTimeoutEvent(int clientFd, const std::string& typedefName);
    int                         initializeSelfPipe(void);
    void                        checkAndHandleTimeouts(void);
    std::vector<int>            getTimedOutFds();
    int                         updateTimeoutEvents(std::vector<t_epollSwitch>& epollSwitch);
    void                        removeTimeoutEvent(int fdToRemove);

    //Update Timeout methods
    void                        createTimeoutUpdatesMap(const std::vector<t_epollSwitch>& epollSwitch, 
                                     std::map<int, std::time_t>& timeoutUpdates);
    void                        updateExistingTimeoutEvents(std::priority_queue<t_timeOutEvent>& newTimeoutEvents, 
                                         std::map<int, std::time_t>& timeoutUpdates); 
    void                        addNewTimeoutEvents(std::priority_queue<t_timeOutEvent>& newTimeoutEvents, 
                                 const std::map<int, std::time_t>& timeoutUpdates);   

public:
    friend std::ostream& operator<<(std::ostream& os, const Server & server);
};


#endif
