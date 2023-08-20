/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequestDELETE.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: znogueir <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/20 14:41:37 by znogueir          #+#    #+#             */
/*   Updated: 2023/08/20 14:41:40 by znogueir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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
