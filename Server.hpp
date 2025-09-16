/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: temil-da <temil-da@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/15 17:01:07 by temil-da          #+#    #+#             */
/*   Updated: 2025/09/15 19:08:24 by temil-da         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "Channel.hpp"
#include "Client.hpp"

class	Server{
private:
	std::map<std::string, CommandHandler>	_cmdMap;
	std::string	_serverName;
	std::string	_serverPass;
public:
	Server(std::string name, std::string pass);
	Server(const Server& other);
	Server&	operator=(const Server& other);
	~Server();

	const std::string&	getName();
	const std::string&	getPass();

	std::string	processInput(Client& client, std::map<std::string, Channel>& channels,
							std::map<int, Client>& clients,
							std::string& buf);
};
