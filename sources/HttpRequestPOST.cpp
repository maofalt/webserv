#include "HttpRequestBase.hpp"
#include "HttpRequestPOST.hpp"

HttpRequestPOST::HttpRequestPOST() {
}

HttpRequestPOST::HttpRequestPOST(int tmp_holder) {
    (void)tmp_holder;
}

HttpRequestPOST::HttpRequestPOST(const HttpRequestPOST & other) : HttpRequestBase(other)
{
    
    (void)other;
}

HttpRequestPOST	&HttpRequestPOST::operator=(const HttpRequestPOST & other) {
    (void)other;
    return *this;
}
