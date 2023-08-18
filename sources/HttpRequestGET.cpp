#include "HttpRequestBase.hpp"
#include "HttpRequestGET.hpp"

HttpRequestGET::HttpRequestGET() {
}

HttpRequestGET::HttpRequestGET(int tmp_holder){
    (void)tmp_holder;
}

HttpRequestGET::HttpRequestGET(const HttpRequestGET & other) : HttpRequestBase(other)
{
    (void)other;
}

HttpRequestGET	&HttpRequestGET::operator=(const HttpRequestGET & other) {
    (void)other;
    return *this;
}
