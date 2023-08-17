#ifndef SERVER_HPP
#define SERVER_HPP

# include <iostream>

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