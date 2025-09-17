/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: temil-da <temil-da@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/15 17:01:07 by temil-da          #+#    #+#             */
/*   Updated: 2025/09/17 17:32:22 by temil-da         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "Channel.hpp"
#include "Client.hpp"

class	Server{
public:
	typedef std::string (Server::*CommandHandler)(
		Client&,
		std::istringstream&
	);
	Server(std::string name, std::string pass);
	Server(const Server& other);
	Server&	operator=(const Server& other);
	~Server();

	std::string	processInput(Client& client, std::string& buf);
private:
	std::map<int, Client>					_clients;
	std::map<std::string, Channel>			_channels;
	std::map<std::string, CommandHandler>	_cmdMap;
	std::string								_serverName;
	std::string								_serverPass;

	std::string	handlePass(Client& client, std::istringstream& str);
	std::string	handleNick(Client& client, std::istringstream& str);
	std::string	handleUser(Client& client, std::istringstream& str);
	std::string	handleJoin(Client& client, std::istringstream& str);
	std::string	handlePrivmsg(Client& client, std::istringstream& str);
	std::string	handlePing(Client& client, std::istringstream& str);
	std::string	handlePong(Client& client, std::istringstream& str);
	std::string	handlePart(Client& client, std::istringstream& str);
	std::string	handleQuit(Client& client, std::istringstream& str);
	std::string	handleKick(Client& client, std::istringstream& str);
	std::string	handleInvite(Client& client, std::istringstream& str);
	std::string	handleTopic(Client& client, std::istringstream& str);
	std::string	handleMode(Client& client, std::istringstream& str);
};
