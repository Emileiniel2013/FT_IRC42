/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: temil-da <temil-da@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/15 17:02:14 by temil-da          #+#    #+#             */
/*   Updated: 2025/09/26 14:34:42 by temil-da         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Utils.hpp"
#include "ErrorCodes.hpp"
#include <sstream>
#include <vector>
#include <sys/socket.h>

Server::Server(std::string name, std::string pass) : _serverName(name), _serverPass(pass) {
	this->_cmdMap["PASS"] = &handlePass;
	this->_cmdMap["NICK"] = &handleNick;
	this->_cmdMap["USER"] = &handleUser;
	this->_cmdMap["JOIN"] = &handleJoin;
	this->_cmdMap["PRIVMSG"] = &handlePrivmsg;
	this->_cmdMap["PING"] = &handlePing;
	this->_cmdMap["PONG"] = &handlePong;
	this->_cmdMap["PART"] = &handlePart;
	this->_cmdMap["QUIT"] = &handleQuit;
	this->_cmdMap["KICK"] = &handleKick;
	this->_cmdMap["INVITE"] = &handleInvite;
	this->_cmdMap["TOPIC"] = &handleTopic;
	this->_cmdMap["MODE"] = &handleMode;
}

Server::Server(const Server& other) : _serverName(other._serverName), _serverPass(other._serverPass), _cmdMap(other._cmdMap) {}

Server&	Server::operator=(const Server& other) {
	if (this != &other){
		this->_serverName = other._serverName;
		this->_serverPass = other._serverPass;
		this->_cmdMap = other._cmdMap;
	}
	return *this;
}

Server::~Server() {}

void	Server::createChannel(Client& creator, const std::string& chName){
	_channels[chName] = Channel(chName);
	_channels[chName].addOperator(creator.getId());
	creator.addChannel(chName);
	sendTopic(creator, _channels[chName]);
	sendNames(creator, _channels[chName]);
}


void	Server::broadcastJoin(Client& client, Channel& ch) {
	std::string	msg = ":" + client.getPrefix() + this->_serverName +
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

void	Server::broadcastMessage(Client& sender, Channel& ch, const std::string message){
	std::string	msg = ":" + sender.getPrefix() + this->_serverName + 
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

void	Server::sendError(Client& client, int errCode, const std::string& target, const std::string& text){
	std::ostringstream oss;
	oss << ":" << this->_serverName << " " << errCode << " "
		<< client.getReplyNick();
	if (!target.empty())
		oss << " " << target;
	oss << " :" << text << "\r\n";
	std::string	msg = oss.str();
	send(client.getId(), msg.c_str(), msg.size(), 0);
}

void	Server::registerUser(Client& client){
	if (client.getAuth())
		return ;
	if (!client.getPassOk())
		return ;
	if (!client.getNickOk() || !client.getUserOk())
		return ;
	client.setAuth(true);
	std::string	msg = ":" + this->_serverName + " 001 " + client.getNick() + " :WELCOME TO THE GREEN AVENGERS IRC NETWORK, "
		+ client.getNick() + "!" + client.getUser() + "@" + _serverName + "\r\n";
	send(client.getId(), msg.c_str(), msg.size(), 0);
}


void	Server::processInput(Client& client, std::string& buf) 
{
	std::string			cmd;
	std::istringstream	keyword(buf);

	keyword >> cmd;
	auto	it = this->_cmdMap.find(cmd);
	if (it == this->_cmdMap.end()){
		sendError(client, ERR_UNKNOWNCOMMAND, cmd, "Unknown command");
		return ;
	}
	if (cmd != "NICK" && cmd != "PASS" && cmd != "USER" && !client.getAuth()){
		sendError(client, ERR_NOTREGISTERED, "", "You have not registered");
		return ;
	}
	(this->*(it->second))(client, keyword);
}
