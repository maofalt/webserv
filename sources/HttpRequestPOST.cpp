/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequestPOST.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: znogueir <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/20 14:41:29 by znogueir          #+#    #+#             */
/*   Updated: 2023/08/20 14:41:30 by znogueir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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
