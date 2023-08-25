/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgarrigo <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/24 21:48:54 by rgarrigo          #+#    #+#             */
/*   Updated: 2023/08/24 23:09:55 by rgarrigo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPRESPOND_HPP
# define HTTPRESPOND_HPP

# include "HttpRequest.hpp"
# include "Logger.hpp"

class HttpResponse
{
	private:
	// Attributes
		const HttpRequest					*_request;
		std::string							_uri;

		std::string							_protocol;
		std::string							_status;
		std::map<std::string, std::string>	_fields;

		std::string							_content;

	// Static
		static std::map<std::string, std::string>	_description;
		static std::map<std::string, std::string>	_content_type;

	public:
	// Constructors
		HttpResponse(void);
		HttpResponse(HttpResponse const &rhs);
		HttpResponse(HttpRequest const *request);

	// Destructor
		~HttpResponse(void);

	// Operators
		HttpResponse	&operator=(HttpResponse const &rhs);

	// Methods
		int		respond(int fd, std::string status);
		void	setRequest(HttpRequest const *request);
};

#endif
