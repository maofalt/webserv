#ifndef HTTPREQUESTGET_HPP
# define HTTPREQUESTGET_HPP

#include "HttpRequestBase.hpp"

class	HttpRequestGET : public HttpRequestBase
{
	public :
		HttpRequestGET();
		HttpRequestGET(/*args*/);
		HttpRequestGET(const HttpRequestGET & other);

		HttpRequestGET	&operator=(const HttpRequestGET & other);

		// Methods;
	private :
		
};

#endif
