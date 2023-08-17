#ifndef HTTPREQUESTBASE_HPP
#define HTTPREQUESTBASE_HPP

# include <iostream>

class HttpRequestBase {
private:
    // Private members
    obj;
public:
    // Default constructor
    HttpRequestBase();

    // Copy constructor
    HttpRequestBase(const HttpRequestBase& other);

    // Copy assignment operator
    HttpRequestBase& operator=(const HttpRequestBase& other);

    // Destructor
    ~HttpRequestBase();
};

std::ostream& operator<<(std::ostream& os, const HttpRequestBase & other);
#endif