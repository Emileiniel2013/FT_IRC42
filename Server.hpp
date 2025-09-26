/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: temil-da <temil-da@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/15 17:01:07 by temil-da          #+#    #+#             */
/*   Updated: 2025/09/26 14:49:25 by temil-da         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "Channel.hpp"
#include "Client.hpp"

class	Server{
public:
	typedef void (Server::*CommandHandler)(
		Client&,
		std::istringstream&
	);
	std::map<int, Client>					_clients;
	std::map<std::string, Channel>			_channels;
	Server(std::string name, std::string pass);
	Server(const Server& other);
	Server&	operator=(const Server& other);
	~Server();

	void	processInput(Client& client, std::string& buf);
private:
	std::map<std::string, CommandHandler>	_cmdMap;
	std::string								_serverName;
	std::string								_serverPass;

	void	createChannel(Client& creator, const std::string& chName);
	void	registerUser(Client& client);
	void	broadcastJoin(Client& client, Channel& ch);
	void	sendTopic(Client& client, Channel& ch);
	void	sendNames(Client& client, Channel& ch);
	void	broadcastMessage(Client& sender, Channel& ch, const std::string& message);
	bool	isInChannel(const std::string& target, Channel& ch);
	bool	isOnServer(const std::string& target);
	int		getIdFromNick(const std::string& target);
	void	sendError(Client& client, int errCode, const std::string& target, const std::string& text);
	void	handlePass(Client& client, std::istringstream& str);
	void	handleNick(Client& client, std::istringstream& str);
	void	handleUser(Client& client, std::istringstream& str);
	void	handleJoin(Client& client, std::istringstream& str);
	void	handlePrivmsg(Client& client, std::istringstream& str);
	void	handlePing(Client& client, std::istringstream& str);
	void	handlePong(Client& client, std::istringstream& str);
	void	handlePart(Client& client, std::istringstream& str);
	void	handleQuit(Client& client, std::istringstream& str);
	void	handleKick(Client& client, std::istringstream& str);
	void	handleInvite(Client& client, std::istringstream& str);
	void	handleTopic(Client& client, std::istringstream& str);
	void	handleMode(Client& client, std::istringstream& str);
};
