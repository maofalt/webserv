#ifndef HTTPREQUESTDELETE_HPP
# define HTTPREQUESTDELETE_HPP

#include "HttpRequestBase.hpp"

class	HttpRequestDELETE : public HttpRequestBase
{
	public :
		HttpRequestDELETE();
		HttpRequestDELETE(int tmp_holder);
		HttpRequestDELETE(const HttpRequestBase & Base);
		HttpRequestDELETE(const HttpRequestDELETE & other);

		HttpRequestDELETE	&operator=(const HttpRequestDELETE & other);

		// Methods;
	private :
		
};

#endif
