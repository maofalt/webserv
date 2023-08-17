#ifndef CLIENTHANDLER_HPP
#define CLIENTHANDLER_HPP

# include <iostream>

class ClientHandler {
private:
    // Private members
    obj;
public:
    // Default constructor
    ClientHandler();

    // Copy constructor
    ClientHandler(const ClientHandler& other);

    // Copy assignment operator
    ClientHandler& operator=(const ClientHandler& other);

    // Destructor
    ~ClientHandler();
};

std::ostream& operator<<(std::ostream& os, const ClientHandler & other);
#endif