/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/03/11 01:18:42 by rgarrigo          #+#    #+#             */
/*   Updated: 2023/08/17 18:19:48 by motero           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

    Server::Server() {
        
    }
    
    Server::Server(const Server& other) {
        
    }
    
    Server &Server::operator=(const Server& other) {
        
    }
    
    Server::~Server() {
        
    }
    

    void    Server::setUpSocket(int port) {
        
    }
    
    int     Server::setUpEpoll(int sock_listen) {
        
    }
    
    int     Server::accept_new_client(int sock_listen) {
        
    }
    
    void    Server::handle_client_data(int client_fd) {
        
    }
    

    void Server::start(){
        
    }
    
    void Server::stop(){
        
    }
    

    void Server::loadConfig(const std::string& configPath) {
        
    }
    std::ostream& operator<<(std::ostream& os, const Server & server);