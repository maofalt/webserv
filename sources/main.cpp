/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/28 20:22:00 by rgarrigo          #+#    #+#             */
/*   Updated: 2023/08/18 19:27:23 by motero           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"


int main(int ac, char **av) {

    Server server;

    if (ac > 1)
        server.loadConfig(av[1]);  // If you have a configuration file.
    else
        server.loadDefaultConfig();

    // try {
    //     server.start();  // Starts listening on all ports and enters event loop.    
    // } catch (const std::exception& e) {
    //     std::cerr << e.what() << std::endl;
    //     std::cerr << "error errror." << std::endl;
    //     return 1;
    // }

    return 0;
}
