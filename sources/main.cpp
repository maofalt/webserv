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

#include "Config.hpp"
#include "Server.hpp"
# include "Logger.hpp"


int main(int ac, char **av) {

    Server server;
    Logger* logger;

    try {
        logger = Logger::getInstance(10 * 1024 * 1024);
        logger->captureStdout();
        logger->captureStderr();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    
    if (ac > 1)
        server.loadConfig(av[1]);  // If you have a configuration file.
    else
        server.loadDefaultConfig();

    if (DISPLAY_CONF)
        std::cout << server.getConfig() << std::endl;

    try {
        server.start();  // Starts listening on all ports and enters event loop.    
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        std::cerr << "error errror." << std::endl;
        Logger::cleanup();
        return 1;
    }

    logger->releaseStdout();
    logger->releaseStderr();
    Logger::cleanup();
    return 0;
}
