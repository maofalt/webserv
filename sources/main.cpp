/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/28 20:22:00 by rgarrigo          #+#    #+#             */
/*   Updated: 2023/08/27 18:18:49 by motero           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"
#include "Server.hpp"
#include "Logger.hpp"


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
    
    //Logger::getInstance()->log(Logger::INFO, "Starting server...", __FILE__, __LINE__);
    INFO_LOG("Starting server...");
    DEBUG_LOG("Testing DEBUG log");
    DEBUG_LOG_DETAILED("Testing DEBUG_DETAILED log");
    WARN_LOG("Testing WARN log");
    ERROR_LOG("Testing ERROR log");
    TRACE_LOG("In the distant future, the year 4045, humanity had reached technological "
          "heights that we, in the early 21st century, could only dream of. "
          "Interstellar travel was not just possible, but commonplace. People lived "
          "on multiple planets, and our society had evolved in ways that would be "
          "utterly foreign to us.\n"
          
          "However, despite all the technological marvels and advancements, the human "
          "heart remained unchanged. People still loved, hated, laughed, cried, and hoped. "
          "The essence of what it meant to be human had endured through the ages. Some argued "
          "that it was our unchanging nature that had allowed us to survive and thrive, while "
          "others believed that it was a testament to our fundamental limitations.\n"
          
          "One day, a historian named Lila made a discovery. Buried deep within an old server "
          "from the 21st century, she found records of conversations, dreams, hopes, and fears of "
          "people from that era. These weren't famous personalities or influential figures, but "
          "everyday individuals discussing their daily lives, their aspirations, and their challenges. "
          "The content was raw, unfiltered, and profoundly human.\n"
          
          "Lila spent months studying these records, trying to piece together the world as it was then. "
          "She felt an overwhelming sense of connection to these long-lost souls. The details of their "
          "lives might have been different, but the emotions, the struggles, the joys, and the pains "
          "were all too familiar. It was a humbling reminder that no matter how much the world changes, "
          "the human experience remains fundamentally the same.\n"
          
          "Inspired by her findings, Lila decided to create a museum. Not just any museum, but one dedicated "
          "to preserving the memories and stories of ordinary people from the past. Visitors could walk through "
          "the exhibits and listen to snippets of conversations, read old messages, and immerse themselves in the "
          "lives of those who lived centuries before. It was a tribute to the timeless nature of the human spirit, "
          "a bridge between the past and the present, reminding everyone that while circumstances change, our core "
          "essence does not.");

    if (ac > 1)
        server.loadConfig(av[1]);  // If you have a configuration file.
    else
        server.loadDefaultConfig();

    // if (DISPLAY_CONF)
    //     std::cout << server.getConfig() << std::endl;
    DEBUG_CONFIG("Config log test: \n");

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