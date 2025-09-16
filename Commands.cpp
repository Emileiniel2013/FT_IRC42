/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Commands.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: temil-da <temil-da@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/15 17:05:32 by temil-da          #+#    #+#             */
/*   Updated: 2025/09/16 12:48:42 by temil-da         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Commands.hpp"

void	initCmdTable(std::map<std::string, CommandHandler>& table){
	table["PASS"] = &handlePass;
	table["NICK"] = &handleNick;
	table["USER"] = &handleUser;
	table["JOIN"] = &handleJoin;
	table["PRIVMSG"] = &handlePrivmsg;
	table["PING"] = &handlePing;
	table["PING"] = &handlePong;
	table["PART"] = &handlePart;
	table["QUIT"] = &handleQuit;
	table["KICK"] = &handleKick;
	table["INVITE"] = &handleInvite;
	table["TOPIC"] = &handleTopic;
	table["MODE"] = &handleMode;
}

std::string	handlePass(Client& client, std::map<std::string, Channel>& channles, std::map<int, Client>& clients, std::istringstream& in) {
	
}

std::string	handleNick(Client& client, std::map<std::string, Channel>& channles, std::map<int, Client>& clients, std::istringstream& in) {return "";}

std::string	handleUser(Client& client, std::map<std::string, Channel>& channles, std::map<int, Client>& clients, std::istringstream& in) {return "";}

std::string	handleJoin(Client& client, std::map<std::string, Channel>& channles, std::map<int, Client>& clients, std::istringstream& in) {return "";}

std::string	handlePrivmsg(Client& client, std::map<std::string, Channel>& channles, std::map<int, Client>& clients, std::istringstream& in) {return "";}

std::string	handlePing(Client& client, std::map<std::string, Channel>& channles, std::map<int, Client>& clients, std::istringstream& in) {return "";}

std::string	handlePong(Client& client, std::map<std::string, Channel>& channles, std::map<int, Client>& clients, std::istringstream& in) {return "";}

std::string	handlePart(Client& client, std::map<std::string, Channel>& channles, std::map<int, Client>& clients, std::istringstream& in) {return "";}

std::string	handleQuit(Client& client, std::map<std::string, Channel>& channles, std::map<int, Client>& clients, std::istringstream& in) {return "";}

std::string	handleKick(Client& client, std::map<std::string, Channel>& channles, std::map<int, Client>& clients, std::istringstream& in) {return "";}

std::string	handleInvite(Client& client, std::map<std::string, Channel>& channles, std::map<int, Client>& clients, std::istringstream& in) {return "";}

std::string	handleTopic(Client& client, std::map<std::string, Channel>& channles, std::map<int, Client>& clients, std::istringstream& in) {return "";}

std::string	handleMode(Client& client, std::map<std::string, Channel>& channles, std::map<int, Client>& clients, std::istringstream& in) {return "";}
