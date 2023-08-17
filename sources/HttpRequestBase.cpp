#include "HttpRequestBase.hpp"
#include "HttpRequestGET.hpp"
#include "HttpRequestPOST.hpp"
#include "HttpRequestDELETE.hpp"

HttpRequestBase::HttpRequestBase() {
}

HttpRequestBase::HttpRequestBase(const HttpRequestBase& other) {
}

HttpRequestBase& HttpRequestBase::operator=(const HttpRequestBase& other) {
}

HttpRequestBase::~HttpRequestBase() {
}

HttpRequestBase *createGETRequest() {
	return new HttpRequestGET;
}

HttpRequestBase *createPOSTRequest() {
	return new HttpRequestPOST;
}

HttpRequestBase *createDELETERequest() {
	return new HttpRequestDELETE;
}

HttpRequestBase *HttpRequestBase::createRequestObj(const std::string RequestType) {
	RequestsTab requestsTab[] = {
        {"GET", &createGETRequest},
        {"POST", &createPOSTRequest},
        {"DELETE", &createDELETERequest},
        {"", NULL}
    };

	for (int i=0; requestsTab[i].createRequest != NULL; i++)
	{
		if (requestsTab[i].type == RequestType)
			return requestsTab[i].createRequest();
	}
	// return ErrorClass with message and potentially error page or something, but for now NULL;
	return NULL;
}

std::ostream& operator<<(std::ostream& os, const HttpRequestBase & other) {
}