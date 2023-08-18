#include "HttpRequestBase.hpp"
#include "HttpRequestPOST.hpp"

HttpRequestPOST::HttpRequestPOST() : HttpRequestBase() {
}

HttpRequestPOST::HttpRequestPOST(int tmp_holder) : HttpRequestBase() {
    (void)tmp_holder;
}

HttpRequestPOST::HttpRequestPOST(const HttpRequestBase & Base) : HttpRequestBase(Base) {
}

HttpRequestPOST::HttpRequestPOST(const HttpRequestPOST & other) : HttpRequestBase(other)
{
    (void)other;
}

HttpRequestPOST	&HttpRequestPOST::operator=(const HttpRequestPOST & other) {
    (void)other;
    return *this;
}
