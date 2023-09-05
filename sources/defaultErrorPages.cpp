/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   defaultErrorPages.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgarrigo <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/02 00:09:15 by rgarrigo          #+#    #+#             */
/*   Updated: 2023/09/02 00:29:46 by rgarrigo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpResponse.hpp"

std::map<std::string, std::string>	HttpResponse::getDefaultErrorPages(void)
{
	std::map<std::string, std::string>	defaultErrorPages;

	defaultErrorPages["400"] = std::string(
		"<!DOCTYPE html>\r\n"
		"<html lang=\"en\">\r\n"
		"<head>\r\n"
		"	<meta charset=\"utf-8\" />\r\n"
		"	<title>400 - Bad request</title>\r\n"
		"</head>\r\n"
		"<body>\r\n"
		"	<h1>Error 400 - Bad request</h1>\r\n"
		"	<img src=\"../images/400.jpg\" alt=\"Error 400 - Bad request\" id=\"bad request\"/>\r\n"
		"</body>\r\n"
		"</html>\r\n");

	defaultErrorPages["404"] = std::string(
		"<!DOCTYPE html>\r\n"
		"<html lang=\"en\">\r\n"
		"<head>\r\n"
		"	<meta charset=\"utf-8\" />\r\n"
		"	<title>404 - Not found</title>\r\n"
		"</head>\r\n"
		"<body>\r\n"
		"	<h1>Error 404 - Page not found</h1>\r\n"
		"	<img src=\"../images/404.jpg\" alt=\"Error 404 - Not found\" id=\"not found\"/>\r\n"
		"</body>\r\n"
		"</html>\r\n");

	defaultErrorPages["405"] = std::string(
		"<!DOCTYPE html>\r\n"
		"<html lang=\"en\">\r\n"
		"<head>\r\n"
		"	<meta charset=\"utf-8\" />\r\n"
		"	<title>405 - Method not allowed</title>\r\n"
		"</head>\r\n"
		"<body>\r\n"
		"	<h1>Error 405 - Method not allowed</h1>\r\n"
		"	<img src=\"../images/405.jpg\" alt=\"Error 405 - Method not allowed\" id=\"not allowed\"/>\r\n"
		"</body>\r\n"
		"</html>\r\n");

	defaultErrorPages["500"] = std::string(
	"<!DOCTYPE html>\r\n"
	"<html lang=\"en\">\r\n"
	"<head>\r\n"
	"	<meta charset=\"utf-8\" />\r\n"
	"	<title>500 - Internal server error</title>\r\n"
	"</head>\r\n"
	"<body>\r\n"
	"	<h1>Error 500 - Internal server error</h1>\r\n"
	"	<img src=\"../images/500.jpg\" alt=\"Error 500 - Internal server error\" id=\"internal error\"/>\r\n"
	"</body>\r\n"
	"</html>\r\n");

	return (defaultErrorPages);
}

std::map<std::string, std::string>	HttpResponse::_defaultErrorPages = HttpResponse::getDefaultErrorPages();
