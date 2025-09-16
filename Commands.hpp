/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Commands.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: temil-da <temil-da@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/15 17:05:01 by temil-da          #+#    #+#             */
/*   Updated: 2025/09/16 12:51:13 by temil-da         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "Channel.hpp"
#include "Client.hpp"
#include <sstream>
#include <map>
#include <string>

typedef std::string (*CommandHandler)(
	Client&,
	std::map<std::string, Channel>&,
	std::map<int, Client>&,
	std::istringstream&
);

void	initCmdTable(std::map<std::string, CommandHandler>& table);

std::string	handlePass(Client& client, std::map<std::string, Channel>& channels, std::map<int, Client>& clients, std::istringstream& str);
std::string	handleNick(Client& client, std::map<std::string, Channel>& channels, std::map<int, Client>& clients, std::istringstream& str);
std::string	handleUser(Client& client, std::map<std::string, Channel>& channels, std::map<int, Client>& clients, std::istringstream& str);
std::string	handleJoin(Client& client, std::map<std::string, Channel>& channels, std::map<int, Client>& clients, std::istringstream& str);
std::string	handlePrivmsg(Client& client, std::map<std::string, Channel>& channels, std::map<int, Client>& clients, std::istringstream& str);
std::string	handlePing(Client& client, std::map<std::string, Channel>& channels, std::map<int, Client>& clients, std::istringstream& str);
std::string	handlePong(Client& client, std::map<std::string, Channel>& channels, std::map<int, Client>& clients, std::istringstream& str);
std::string	handlePart(Client& client, std::map<std::string, Channel>& channels, std::map<int, Client>& clients, std::istringstream& str);
std::string	handleQuit(Client& client, std::map<std::string, Channel>& channels, std::map<int, Client>& clients, std::istringstream& str);
std::string	handleKick(Client& client, std::map<std::string, Channel>& channels, std::map<int, Client>& clients, std::istringstream& str);
std::string	handleInvite(Client& client, std::map<std::string, Channel>& channels, std::map<int, Client>& clients, std::istringstream& str);
std::string	handleTopic(Client& client, std::map<std::string, Channel>& channels, std::map<int, Client>& clients, std::istringstream& str);
std::string	handleMode(Client& client, std::map<std::string, Channel>& channels, std::map<int, Client>& clients, std::istringstream& str);
