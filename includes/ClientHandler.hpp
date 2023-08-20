#ifndef CLIENTHANDLER_HPP
#define CLIENTHANDLER_HPP

# include <iostream>

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
  int             client_fd;
  HttpRequestBase request;

public:
    // Default constructor
    ClientHandler();

    // Copy constructor
    ClientHandler(const ClientHandler& other);

    // Copy assignment operator
    ClientHandler& operator=(const ClientHandler& other);

    // Destructor
    ~ClientHandler();

    ClientHandler(int fd) : client_fd(fd) {}

    void readData();

    void processData();

    void writeResponse();

    bool isRequestComplete();

    void closeConnection();

};

std::ostream& operator<<(std::ostream& os, const ClientHandler & other);
#endif