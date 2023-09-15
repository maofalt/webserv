/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   defaultErrorPages.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgarrigo <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/02 00:09:15 by rgarrigo          #+#    #+#             */
/*   Updated: 2023/09/15 18:51:27 by rgarrigo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpResponse.hpp"

std::map<std::string, std::string>	HttpResponse::getDefaultErrorPages(void)
{
	std::map<std::string, std::string>	defaultErrorPages;

	defaultErrorPages["400"] = std::string(
		"<!DOCTYPE html>\n"
		"<html lang=\"en\">\n"
		"<head>\n"
		"	<meta charset=\"utf-8\" />\n"
		"	<title>400 - Bad request</title>\n"
		"	<link rel=\"stylesheet\" href=\"../css/style_errors.css\" />\n"
		"</head>\n"
		"<body>\n"
		"	<h1>Error 400</h1>\n"
		"	<img src=\"../images2/400.jpg\" alt=\"Error 400 - Bad request\" id=\"bad request\"/>\n"
		"	<h1>Bad request</h1>\n"
		"</body>\n"
		"</html>\n");

	defaultErrorPages["401"] = std::string(
		"<!DOCTYPE html>\r\n"
		"<html lang=\"en\">\r\n"
		"<head>\r\n"
		"	<meta charset=\"utf-8\" />\r\n"
		"	<title>401 - Unauthorized</title>\r\n"
		"</head>\r\n"
		"<body>\r\n"
		"	<h1>Error 401</h1>\r\n"
		"	<img src=\"../images2/401.jpg\" alt=\"Error 401 - Unauthorized\" id=\"unauthorized\"/>\r\n"
		"	<h1>Unauthorized</h1>\n"
		"</body>\r\n"
		"</html>\r\n");

	defaultErrorPages["403"] = std::string(
		"<!DOCTYPE html>\n"
		"<html lang=\"en\">\n"
		"<head>\n"
		"	<meta charset=\"utf-8\" />\n"
		"	<title>403 - Forbidden</title>\n"
		"	<link rel=\"stylesheet\" href=\"../css/style_errors.css\" />\n"
		"</head>\n"
		"<body>\n"
		"	<h1>Error 403</h1>\n"
		"	<img src=\"../images2/403.jpg\" alt=\"Error 403 - Forbidden\" id=\"forbidden\"/>\n"
		"	<h1>Forbidden</h1>\n"
		"</body>\n"
		"</html>\n");

	defaultErrorPages["404"] = std::string(
		"<!DOCTYPE html>\n"
		"<html lang=\"en\">\n"
		"<head>\n"
		"	<meta charset=\"utf-8\" />\n"
		"	<title>404 - Not found</title>\n"
		"	<link rel=\"stylesheet\" href=\"../css/style_errors.css\" />\n"
		"</head>\n"
		"<body>\n"
		"	<h1>Error 404</h1>\n"
		"	<img src=\"../images2/404.jpg\" alt=\"Error 404 - Not found\" id=\"not found\"/>\n"
		"	<h1>Page not found</h1>\n"
		"</body>\n"
		"</html>\n");

	defaultErrorPages["405"] = std::string(
		"<!DOCTYPE html>\n"
		"<html lang=\"en\">\n"
		"<head>\n"
		"	<meta charset=\"utf-8\" />\n"
		"	<title>405 - Method not allowed</title>\n"
		"	<link rel=\"stylesheet\" href=\"../css/style_errors.css\" />\n"
		"</head>\n"
		"<body>\n"
		"	<h1>Error 405</h1>\n"
		"	<img src=\"../images2/405.jpg\" alt=\"Error 405 - Method not allowed\" id=\"not allowed\"/>\n"
		"	<h1>Method not allowed</h1>\n"
		"</body>\n"
		"</html>\n");

	defaultErrorPages["408"] = std::string(
		"<!DOCTYPE html>\n"
		"<html lang=\"en\">\n"
		"<head>\n"
		"	<meta charset=\"utf-8\" />\n"
		"	<title>408 - Request Timeout</title>\n"
		"	<link rel=\"stylesheet\" href=\"../css/style_errors.css\" />\n"
		"</head>\n"
		"<body>\n"
		"	<h1>Error 408</h1>\n"
		"	<img src=\"../images2/408.jpg\" alt=\"Error 408 - Request Timeout\" id=\"request timeout\"/>\n"
		"	<h1>Request Timeout</h1>\n"
		"</body>\n"
		"</html>\n");

	defaultErrorPages["411"] = std::string(
		"<!DOCTYPE html>\n"
		"<html lang=\"en\">\n"
		"<head>\n"
		"	<meta charset=\"utf-8\" />\n"
		"	<title>411 - Length required</title>\n"
		"	<link rel=\"stylesheet\" href=\"../css/style_errors.css\" />\n"
		"</head>\n"
		"<body>\n"
		"	<h1>Error 411</h1>\n"
		"	<img src=\"../images2/411.jpg\" alt=\"Error 411 - Length required\" id=\"length required\"/>\n"
		"	<h1>Length required</h1>\n"
		"</body>\n"
		"</html>\n");

	defaultErrorPages["413"] = std::string(
		"<!DOCTYPE html>\n"
		"<html lang=\"en\">\n"
		"<head>\n"
		"	<meta charset=\"utf-8\" />\n"
		"	<title>413 - Request entity too large</title>\n"
		"	<link rel=\"stylesheet\" href=\"../css/style_errors.css\" />\n"
		"</head>\n"
		"<body>\n"
		"	<h1>Error 413</h1>\n"
		"	<img src=\"../images2/413.jpg\" alt=\"Error 413 - Request entity too large\" id=\"entity too large\"/>\n"
		"	<h1>Request entity too large</h1>\n"
		"</body>\n"
		"</html>\n");

	defaultErrorPages["500"] = std::string(
		"<!DOCTYPE html>\n"
		"<html lang=\"en\">\n"
		"<head>\n"
		"	<meta charset=\"utf-8\" />\n"
		"	<title>500 - Internal server error</title>\n"
		"	<link rel=\"stylesheet\" href=\"../css/style_errors.css\" />\n"
		"</head>\n"
		"<body>\n"
		"	<h1>Error 500</h1>\n"
		"	<img src=\"../images2/500.jpg\" alt=\"Error 500 - Internal server error\" id=\"internal error\"/>\n"
		"	<h1>Internal server error</h1>\n"
		"</body>\n"
		"</html>\n");

	defaultErrorPages["501"] = std::string(
		"<!DOCTYPE html>\n"
		"<html lang=\"en\">\n"
		"<head>\n"
		"	<meta charset=\"utf-8\" />\n"
		"	<title>501 - Not implemented</title>\n"
		"	<link rel=\"stylesheet\" href=\"../css/style_errors.css\" />\n"
		"</head>\n"
		"<body>\n"
		"	<h1>Error 501</h1>\n"
		"	<img src=\"../images2/501.jpg\" alt=\"Error 501 - Not implemented\" id=\"not implemented\"/>\n"
		"	<h1>Not implemented</h1>\n"
		"</body>\n"
		"</html>\n");

	defaultErrorPages["505"] = std::string(
		"<!DOCTYPE html>\n"
		"<html lang=\"en\">\n"
		"<head>\n"
		"	<meta charset=\"utf-8\" />\n"
		"	<title>505 - HTTP version not supported</title>\n"
		"	<link rel=\"stylesheet\" href=\"../css/style_errors.css\" />\n"
		"</head>\n"
		"<body>\n"
		"	<h1>Error 505</h1>\n"
		"	<img src=\"../images2/505.jpg\" alt=\"Error 505 - HTTP Version not supported\" id=\"version not supported\"/>\n"
		"	<h1>HTTP version not supported</h1>\n"
		"</body>\n"
		"</html>\n");

	return (defaultErrorPages);
}

std::map<std::string, std::string>	HttpResponse::_defaultErrorPages = HttpResponse::getDefaultErrorPages();
