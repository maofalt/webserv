#include "HttpRequestBase.hpp"
#include "HttpRequestDELETE.hpp"

HttpRequestDELETE::HttpRequestDELETE() : HttpRequestBase() {
}

HttpRequestDELETE::HttpRequestDELETE(int tmp_holder)  : HttpRequestBase() {
    (void)tmp_holder;
}

HttpRequestDELETE::HttpRequestDELETE(const HttpRequestBase & Base) : HttpRequestBase(Base) {
}


HttpRequestDELETE::HttpRequestDELETE(const HttpRequestDELETE & other) : HttpRequestBase(other)
{
    (void)other;
}

HttpRequestDELETE	&HttpRequestDELETE::operator=(const HttpRequestDELETE & other) {
    (void)other;
    return *this;
}
