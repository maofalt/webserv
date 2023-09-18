/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/28 20:22:00 by rgarrigo          #+#    #+#             */
/*   Updated: 2023/09/18 18:23:25 by motero           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"
#include "ConfigValidator.hpp"
#include "Server.hpp"
#include "Logger.hpp"

void printConfigFile(Server& server) {
    std::ostringstream oss;
    oss << server.getConfig();
    DEBUG_CONFIG(oss);
    INFO_LOG("Configuration loaded.");
}

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


//Some logic to be improved here : if we laod the default config, and it fails it will laod the default config again and revalidate it
bool loadAndValidateConfig(Server& server, const char* configFile = NULL) {
    if (configFile) {
        if (server.loadConfig(configFile)) return false;
    } else {
        if (server.loadDefaultConfig()) return false;
    }

    //Validate config, if it fails, load default config and validate it
    printConfigFile(server);
    ConfigValidator validator(server.getValidationFile(), server.getConfig().getConfData(), server.getConfig().getServList());
    if (!validator.validateConfig()) {
        log_message(Logger::ERROR, "Configuration validation failed. Trying default configuration.");
    printConfigFile(server);
        if (server.loadDefaultConfig()) return false;
        printConfigFile(server);
        //destroy validator with validator destructor
        validator.~ConfigValidator();
        ConfigValidator validator(server.getValidationFile(), server.getConfig().getConfData(), server.getConfig().getServList());
        if (!validator.validateConfig()) {
            log_message(Logger::ERROR, "Default Configuration validation failed. Exiting.");
            return false;
        }
        log_message(Logger::INFO, "Default Configuration validation passed.");
    }
    
    // if validate we set the global config
    t_globalConfig globalConfig = validator.getGlobalConfig();
    Config& config = server.getConfig();
    config.setGlobalConfig(globalConfig);
    ClientHandler::setConfig(server.getConfig());
    log_message(Logger::INFO, "Configuration validation passed. Starting server.");
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
