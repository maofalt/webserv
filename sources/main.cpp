/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/28 20:22:00 by rgarrigo          #+#    #+#             */
/*   Updated: 2023/09/05 13:44:17 by motero           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"
#include "Server.hpp"
#include "Logger.hpp"


// int main(int ac, char **av) {

//     Server server;
//     Logger* logger;

//     try {
//         logger = Logger::getInstance(10 * 1024 * 1024);
//         logger->captureStdout();
//         logger->captureStderr();
//     } catch (const std::exception& e) {
//         ERROR_LOG(e.what());
//         return 1;
//     }
//     INFO_LOG("Logger initialized.");
    
//     try {
//         server.loadValidationFile(PATH_INI);
//         //server.setValidationFile(server.getValidationFile());
//     } catch (const std::exception& e) {
//         ERROR_LOG(e.what());
//         return 1;
//     }
    
//     if (ac > 1) {
//         if (server.loadConfig(av[1])) // If you have a configuration file.
//             return 1;
//     }
//     else if (server.loadDefaultConfig())
//         return 1;

//     std::ostringstream oss;
//     oss << server.getConfig();
//     DEBUG_CONFIG(oss);
//     INFO_LOG("Configuration loaded.");

//     try {
//         server.setValidationFile(server.getValidationFile());
//         if(!server.getConfig().validateConfig() ) {
//             log_message(Logger::ERROR, "Configuration validation failed. We'll try default configuration.");
//             if (server.loadDefaultConfig())
//                 return 1;
//             server.setValidationFile(server.getValidationFile());
//             if(server.getConfig().validateConfig()) {
//                log_message(Logger::ERROR, "Configuration validation failed. We'll try default configuration.");
//                 return 1;    
//             }
//         }
//     } catch (const std::exception& e) {
//         ERROR_LOG(e.what());
//         return 1;
//     }


//     try {
//         server.start();  // Starts listening on all ports and enters event loop.    
//     } catch (const std::exception& e) {
        
//         std::cerr << e.what() << std::endl;
//         Logger::cleanup();
//         return 1;
//     }

//     logger->releaseStdout();
//     logger->releaseStderr();
//     Logger::cleanup();
//     return 0;
// }

bool initializeLogger() {
    try {
        Logger* logger = Logger::getInstance(10 * 1024 * 1024);
        logger->captureStdout();
        logger->captureStderr();
        INFO_LOG("Logger initialized.");
        return true;
    } catch (const std::exception& e) {
        ERROR_LOG(e.what());
        return false;
    }
}

bool loadServerValidationFile(Server& server) {
    try {
        server.loadValidationFile(PATH_INI);
        return true;
    } catch (const std::exception& e) {
        ERROR_LOG(e.what());
        return false;
    }
}

bool loadAndValidateConfig(Server& server, const char* configFile = NULL) {
    if (configFile) {
        if (server.loadConfig(configFile)) return false;
    } else {
        if (server.loadDefaultConfig()) return false;
    }
    
     server.setValidationFile(server.getValidationFile());
    if (!server.getConfig().validateConfig()) {
        log_message(Logger::ERROR, "Configuration validation failed. Trying default configuration.");
        if (server.loadDefaultConfig()) return false;
        server.setValidationFile(server.getValidationFile());
        if (!server.getConfig().validateConfig()) return false;
    }
    
    return true;
}

bool startServer(Server& server) {
    try {
        server.start();
        return true;
    } catch (const std::exception& e) {
        ERROR_LOG(e.what());
        return false;
    }
}

int main(int ac, char **av) {
    Server server;

    if (!initializeLogger()) return 1;
    if (!loadServerValidationFile(server)) return 1;
    if (!loadAndValidateConfig(server, (ac > 1) ? av[1] : NULL)) return 1;
    if (!startServer(server)) return 1;

    Logger* logger = Logger::getInstance();
    logger->releaseStdout();
    logger->releaseStderr();
    Logger::cleanup();
    return 0;
}
