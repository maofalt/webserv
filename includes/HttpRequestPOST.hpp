#ifndef HTTPREQUESTPOST_HPP
# define HTTPREQUESTPOST_HPP

#include "HttpRequestBase.hpp"

class	HttpRequestPOST : public HttpRequestBase
{
	public :
		HttpRequestPOST();
		HttpRequestPOST(int tmp_holder);
		HttpRequestPOST(const HttpRequestBase & Base);
		HttpRequestPOST(const HttpRequestPOST & other);

		HttpRequestPOST	&operator=(const HttpRequestPOST & other);

		// Methods;
	private :
		
};

#endif
