#ifndef SERVER_HPP
#define SERVER_HPP

# include <iostream>

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
    // Private members
    obj;
public:
    // Default constructor
    Server();

    // Copy constructor
    Server(const Server& other);

    // Copy assignment operator
    Server& operator=(const Server& other);

    // Destructor
    ~Server();
};

std::ostream& operator<<(std::ostream& os, const Server & other);
#endif