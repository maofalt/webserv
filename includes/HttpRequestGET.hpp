/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequestGET.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: znogueir <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/20 14:42:19 by znogueir          #+#    #+#             */
/*   Updated: 2023/08/20 14:42:21 by znogueir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUESTGET_HPP
# define HTTPREQUESTGET_HPP

#include "HttpRequestBase.hpp"

class	HttpRequestGET : public HttpRequestBase
{
	public :
		HttpRequestGET();
		HttpRequestGET(int tmp_holder);
		HttpRequestGET(const HttpRequestBase & Base);
		HttpRequestGET(const HttpRequestGET & other);

		HttpRequestGET	&operator=(const HttpRequestGET & other);

		// Methods;
		int	respond(int fd, std::string status);
	private :
		
};

#endif
