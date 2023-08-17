#ifndef HTTPREQUESTBASE_HPP
#define HTTPREQUESTBASE_HPP

# include <iostream>

/*
Purpose:            To act as a base class for all HTTP request types.
Attributes/Methods: We already got a good starting point with the 
                    existing HttpRequest class. 
                    Refactor this to make it more generic 
                    (i.e., remove GET-specific behavior).
*/

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