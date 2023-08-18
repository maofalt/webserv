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


int main(void) {

    Server server;
    server.loadConfig("path_to_config");  // If you have a configuration file.
    server.start();  // Starts listening on all ports and enters event loop.

    return 0;
}
