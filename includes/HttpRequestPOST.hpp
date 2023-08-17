#ifndef HTTPREQUESTPOST_HPP
# define HTTPREQUESTPOST_HPP

#include "HttpRequestBase.hpp"

class	HttpRequestPOST : public HttpRequestBase
{
	public :
		HttpRequestPOST();
		HttpRequestPOST(/*args*/);
		HttpRequestPOST(const HttpRequestPOST & other);

		HttpRequestPOST	&operator=(const HttpRequestPOST & other);

		// Methods;
	private :
		
};

#endif
