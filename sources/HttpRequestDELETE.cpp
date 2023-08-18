#include "HttpRequestBase.hpp"
#include "HttpRequestDELETE.hpp"

HttpRequestDELETE::HttpRequestDELETE() {
}

HttpRequestDELETE::HttpRequestDELETE(int tmp_holder) {
    (void)tmp_holder;
}

HttpRequestDELETE::HttpRequestDELETE(const HttpRequestDELETE & other) : HttpRequestBase(other)
{
    (void)other;
}

HttpRequestDELETE	&HttpRequestDELETE::operator=(const HttpRequestDELETE & other) {
    (void)other;
    return *this;
}
