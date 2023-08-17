/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: motero <motero@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/28 20:22:00 by rgarrigo          #+#    #+#             */
/*   Updated: 2023/08/17 18:10:56 by motero           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

//Move this to Server class!
volatile sig_atomic_t	run = 1;

void	signal_handler(int sig)
{
	if (sig == SIGINT)
	run = 0;
}

int main(void) {
    signal(SIGINT, signal_handler);  // This handler should set a global flag or a Server member to indicate server should shut down.

    Server server;
    server.loadConfig("path_to_config");  // If you have a configuration file.
    server.start();  // Starts listening on all ports and enters event loop.

    return 0;
}
