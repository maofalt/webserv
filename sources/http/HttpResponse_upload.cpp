/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse_upload.cpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgarrigo <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/25 01:32:32 by rgarrigo          #+#    #+#             */
/*   Updated: 2023/09/25 01:33:07 by rgarrigo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpResponse.hpp"

int	HttpResponse::_skipLine(std::string::size_type &i)
{
	i = _request->_body.find("\n", i);
	if (i == std::string::npos)
		return (-1);
	i++;
	return (0);
}

int	HttpResponse::_readUploadContentHeader(
	std::string &boundary,
	std::string &filename,
	std::string::size_type &i)
{
	std::string::size_type	j;
	bool					filenameSet;

	filenameSet = false;
	i = _request->_body.find(boundary, i);
	if (i == std::string::npos)
		return (-1);
	i += boundary.size();
	while (1)
	{
		if (_skipLine(i) == -1
			|| _request->_body.find(boundary, i) == i)
			return (-1);
		if (_request->_body.find("\r\n", i) == i
			||  _request->_body.find("\n", i) == i)
			break ;
		if (filenameSet
			|| _request->_body.find("Content-Disposition: form-data; ", i) != i
			|| _request->_body.find("filename=\"", i) > _request->_body.find("\n", i))
			continue ;
		i = _request->_body.find("filename=\"", i);
		if (i == std::string::npos)
			return (-1);
		i += 10;
		j = _request->_body.find("\"", i);
		if (j == std::string::npos)
			return (-1);
		filename = _request->_body.substr(i, j - i);
		filenameSet = true;
	}
	return (_skipLine(i));
}

int	HttpResponse::_readUploadContentBody(
	std::string &boundary,
	std::string &file,
	std::string::size_type &i)
{
	std::string::size_type	j;

	j = _request->_body.find(boundary, i);
	if (j >= i + 2
		&& _request->_body.find("\r\n", j - 2) == j - 2)
		j -= 2;
	else if (j >= i + 1
		&& _request->_body.find("\n", j - 1) == j - 1)
		j--;
	file = _request->_body.substr(i, j - i);
	i = j;
	return (0);
}

int	HttpResponse::_createFile(std::string &file, std::string &path)
{
	std::ofstream	fd;

	fd.open(path.c_str());
	if (!fd.is_open())
		return (ERROR_LOG("File not created"), -1);
	fd.write(file.c_str(), file.size());
	fd.flush();
	fd.close();
	return (0);
}

int	HttpResponse::_uploadFile(void)
{
	std::string				prefix("multipart/form-data; boundary=----");
	std::string				boundary("------");
	std::string				rootUpload;
	std::string				filename;
	std::string				path;
	std::string				file;
	std::string::size_type	i;

	if (_request->_field.at("Content-Type").find(prefix) != 0)
		return (2);
	boundary += _request->_field.at("Content-Type").substr(prefix.size(), std::string::npos);
	rootUpload = _location->_locConfig.at("upload")[1];
	if (*rootUpload.rbegin() != '/')
		rootUpload.push_back('/');
	i = 0;
	while (i != std::string::npos)
	{
		if (_readUploadContentHeader(boundary, filename, i) == -1)
			continue ;
		path = rootUpload + filename;
		if (access(path.c_str(), F_OK) != -1)
			continue ;
		if (_readUploadContentBody(boundary, file, i) == -1)
			continue ;
		_createFile(file, path);
	}
	return (0);
}
