#ifndef CLIENTHANDLER_HPP
#define CLIENTHANDLER_HPP


# include <iostream>
# include <unistd.h>
# include <sys/epoll.h>
# include "HttpRequestBase.hpp"

/*
ClientHandler Class:

Purpose:    To manage communication with an individual client.
Attributes:
            - Socket or file descriptor for the client.
            - An instance of HttpRequest (or later, HttpRequestBase 
              and derived classes) to parse the incoming request.
            - An instance of HttpResponse to prepare and send the response.
Methods:
            - Read from the client.
            - Write to the client.
            - Disconnect/close client connection.
*/

class ClientHandler {
private:
  int             _client_fd;
  HttpRequestBase _request;

public:
    ClientHandler();
    ClientHandler(int fd, HttpRequestBase request);
    ClientHandler(const ClientHandler& other);
    ClientHandler& operator=(const ClientHandler& other);
    // Destructor
    ~ClientHandler();

    //Methods to handle 
    void readData();
    void writeResponse();
    bool isRequestComplete();
    void closeConnection(int epoll_fd);


//Disable cnstructors and assignment operator
private: 
   
};

std::ostream& operator<<(std::ostream& os, const ClientHandler & other);
#endif