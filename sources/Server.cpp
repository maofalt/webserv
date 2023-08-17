/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/03/11 01:18:42 by rgarrigo          #+#    #+#             */
/*   Updated: 2023/08/17 18:17:03 by motero           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"


   
    void    setUpSocket(int port);
    int     setUpEpoll(int sock_listen);
    int     accept_new_client(int sock_listen);
    void    handle_client_data(int client_fd);

    Server();
    Server(const Server& other);
    Server& operator=(const Server& other);
    ~Server();

    void start();
    void stop();

    void loadConfig(const std::string& configPath);
    std::ostream& operator<<(std::ostream& os, const Server & server);