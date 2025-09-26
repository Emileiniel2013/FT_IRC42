/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerHelpers.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: temil-da <temil-da@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/17 22:55:59 by temil-da          #+#    #+#             */
/*   Updated: 2025/09/26 14:51:19 by temil-da         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "ServerHelpers.hpp"
#include <sstream>
#include <sys/socket.h>

std::vector<std::string>	split(const std::string& str, char delim){
	std::vector<std::string>	res;
	std::string					currStr;
	std::istringstream			ss(str);
	while (std::getline(ss, currStr, delim)){
		if (!currStr.empty())
			res.push_back(currStr);
	}
	return res;
}

void						trimPrefix(std::string& str){
	if (!str.empty() && str[0] == ' ')
		str.erase(0, 1);
	if (!str.empty() && str[0] == ':')
		str.erase(0, 1);
}

void	Server::createChannel(Client& creator, const std::string& chName){
	_channels[chName] = Channel(chName);
	_channels[chName].addOperator(creator.getId());
	creator.addChannel(chName);
	broadcastJoin(creator, _channels[chName]);
	sendTopic(creator, _channels[chName]);
	sendNames(creator, _channels[chName]);
}


void	Server::broadcastJoin(Client& client, Channel& ch) {
	std::string	msg = ":" + client.getPrefix() +
		" JOIN :" + ch.getName() + "\r\n";
	ch.broadcast(msg);
}

void	Server::sendTopic(Client& client, Channel& ch){
	std::string	msg;
	if (!ch.getTopic().empty()){
		msg = ":" + this->_serverName + " 332 " + client.getReplyNick() +
			" " + ch.getName() + " :" + ch.getTopic() + "\r\n";
		send(client.getId(), msg.c_str(), msg.size(), 0);
	}
	else{
		msg = ":" + this->_serverName + " 331 " + client.getReplyNick() +
			" " + ch.getName() + " :No topic is set\r\n";
		send(client.getId(), msg.c_str(), msg.size(), 0);
	}
}

void	Server::sendNames(Client& client, Channel& ch){
	std::string	namesList;
	for (int id : ch.getAllMembers()){
		Client&	member = _clients[id];
		if (ch.isOperator(id))
			namesList += "@";
		namesList += member.getNick() + " ";
	}
	if (!namesList.empty())
		namesList.pop_back();
	std::string	msg = ":" + this->_serverName + " 353 " + client.getReplyNick() + 
	" = " + ch.getName() + " :" + namesList + "\r\n";
	send(client.getId(), msg.c_str(), msg.size(), 0);
	msg = ":" + this->_serverName + " 366 " + client.getReplyNick() + 
	" = " + ch.getName() + " :End of NAMES list\r\n";
	send(client.getId(), msg.c_str(), msg.size(), 0);
}

void	Server::broadcastMessage(Client& sender, Channel& ch, const std::string& message){
	std::string	msg = ":" + sender.getPrefix() + 
			" PRIVMSG " + ch.getName() + " :" + message + "\r\n";
	for (int id : ch.getAllMembers()){
		if (id != sender.getId())
			send(id, msg.c_str(), msg.size(), 0);
	}
}

bool	Server::isInChannel(const std::string& target, Channel& ch){
	for (int id : ch.getAllMembers()){
		if (_clients[id].getNick() == target)
			return true ;
	}
	return false ;
}

bool	Server::isOnServer(const std::string& target){
	for (auto it = _clients.begin(); it != _clients.end(); ++it){
		if (it->second.getNick() == target)
			return true ;
	}
	return false ;
}

int		Server::getIdFromNick(const std::string& target) {
	for (auto it = _clients.begin(); it != _clients.end(); ++it)
		if (it->second.getNick() == target)
			return it->second.getId();
	return -1;
}