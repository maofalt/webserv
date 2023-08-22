/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequestPOST.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: znogueir <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/20 14:42:27 by znogueir          #+#    #+#             */
/*   Updated: 2023/08/20 14:42:28 by znogueir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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
