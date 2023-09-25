/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse_cgi.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgarrigo <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/25 01:29:04 by rgarrigo          #+#    #+#             */
/*   Updated: 2023/09/25 15:17:04 by rgarrigo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "FDManager.hpp"
#include "Logger.hpp"
#include "HttpResponse.hpp"

int	HttpResponse::_setEnvCgi(void)
{
	std::ostringstream	variable("");

	variable << "SERVER_SOFTWARE=" << SERVER_NAME << "/" << SERVER_VERSION;
	_envCgi.push_back(variable.str());
	variable.str("");

	variable << "SERVER_NAME=" << _host;
	_envCgi.push_back(variable.str());
	variable.str("");

	variable << "GATEWAY_INTERFACE=CGI/1.1";
	_envCgi.push_back(variable.str());
	variable.str("");

	variable << "REQUEST_METHOD=" << _method;
	_envCgi.push_back(variable.str());
	variable.str("");

	variable << "SCRIPT_NAME=" << _uri;
	_envCgi.push_back(variable.str());
	variable.str("");

	variable << "SCRIPT_FILENAME=" << _path;
	_envCgi.push_back(variable.str());
	variable.str("");

	variable << "REQUEST_FILENAME=" << _path;
	_envCgi.push_back(variable.str());
	variable.str("");

	variable << "QUERY_STRING=" << _queryString;
	_envCgi.push_back(variable.str());
	variable.str("");

	variable << "REDIRECT_STATUS=";
	_envCgi.push_back(variable.str());
	variable.str("");

	if (_request->_field.count("Cookie"))
	{
		variable << "HTTP_COOKIE=" << _request->_field.at("Cookie");
		_envCgi.push_back(variable.str());
		variable.str("");
	}

	if (_method == "POST")
	{
		if (_request->_field.count("Content-Type"))
		{
			variable << "CONTENT_TYPE=" << _request->_field.at("Content-Type");
			_envCgi.push_back(variable.str());
			variable.str("");
		}

		variable << "CONTENT_LENGTH=" << _request->_body.size();
		_envCgi.push_back(variable.str());
		variable.str("");
	}

	return (0);
}

int	HttpResponse::_launchSon(int pipeFdIn[2], int pipeFdOut[2])
{
	close(pipeFdIn[1]);
	close(pipeFdOut[0]);
	if (dup2(pipeFdIn[0], 0) == -1)
		return (close(pipeFdIn[0]), close(pipeFdOut[1]), -1);
	close(pipeFdIn[0]);
	if (dup2(pipeFdOut[1], 1) == -1)
		return (close(pipeFdOut[1]), -1);
	close(pipeFdOut[1]);

	std::string	pathExec;
	char		*argv[3];
	char		*env[100];

	pathExec = _location->_locConfig.at("cgi")[1];
	argv[0] = &pathExec[0];
	if (_path.find("./", 0) == 0)
	{
		_path.erase(0, 2);
		_path.insert(0, "/");
		_path.insert(0, std::getenv("PWD"));
	}
	argv[1] = &_path[0];
	argv[2] = NULL;
	_setEnvCgi();
	int	i = 0;
	for (std::vector<std::string>::iterator it = _envCgi.begin(); i < 99 && it != _envCgi.end(); ++it)
		env[i++] = &((*it)[0]);
	env[i] = NULL;
	chdir(_path.substr(0, _path.find_last_of("/")).c_str());

	FDManager::closeAllFds();
	Logger::cleanup();

	if (execve(argv[0], argv, env) == -1)
		std::exit(1);
	return (0);
}

int	HttpResponse::_launchCgi(void)
{
	int		pid;
	int		pipeFdIn[2];
	int		pipeFdOut[2];

	if (_uploadFileOn)
		_uploadFile();
	if (pipe(pipeFdIn) == -1)
		return (_writeError("500"));
	if (pipe(pipeFdOut) == -1)
		return (close(pipeFdIn[0]), close(pipeFdIn[1]), _writeError("500"));
	pid = fork();
	if (pid == -1)
		return (close(pipeFdIn[0]), close(pipeFdIn[1]), close(pipeFdOut[0]), close(pipeFdOut[1]), _writeError("500"));
	if (pid == 0)
		_launchSon(pipeFdIn, pipeFdOut);
	_pidCgi = pid;
	close(pipeFdIn[0]);
	close(pipeFdOut[1]);
	_fdCgiIn = pipeFdIn[1];
	_fdCgiOut = pipeFdOut[0];
	return (CGI_LAUNCHED);
}
