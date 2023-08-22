/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequestDELETE.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: znogueir <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/20 14:42:32 by znogueir          #+#    #+#             */
/*   Updated: 2023/08/20 14:42:34 by znogueir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUESTDELETE_HPP
# define HTTPREQUESTDELETE_HPP

#include "HttpRequestBase.hpp"

class	HttpRequestDELETE : public HttpRequestBase
{
	public :
		HttpRequestDELETE();
		HttpRequestDELETE(int tmp_holder);
		HttpRequestDELETE(const HttpRequestBase & Base);
		HttpRequestDELETE(const HttpRequestDELETE & other);

		HttpRequestDELETE	&operator=(const HttpRequestDELETE & other);

		// Methods;
	private :
		
};

#endif
