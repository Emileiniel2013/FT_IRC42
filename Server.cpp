/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: temil-da <temil-da@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/15 17:02:14 by temil-da          #+#    #+#             */
/*   Updated: 2025/09/17 18:22:07 by temil-da         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <sstream>

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
		_serverName = other._serverName;
		_serverPass = other._serverPass;
		this->_cmdMap = other._cmdMap;
	}
	return *this;
}

Server::~Server() {}

std::string	Server::processInput(Client& client, std::string& buf) 
{
	std::string			cmd;
	std::istringstream	keyword(buf);

	keyword >> cmd;
	auto	it = this->_cmdMap.find(cmd);
	if (it != this->_cmdMap.end())
		return (this->*(it->second))(client, keyword);
	else
		return ":" + this->_serverName + " 421 " + client.getReplyNick() + " " + cmd + " :Unknown command\r\n";
}

std::string	Server::handlePass(Client& client, std::istringstream& in) {
	std::string	password;
	in >> password;
	
	if (password.empty())
		return ":" + this->_serverName + " 461 " + client.getReplyNick() + " PASS: Not enough parameters\r\n";
	if (client.getAuth())
		return ":" + this->_serverName + " 462 " + client.getReplyNick() + " :You might not reregister\r\n";
	if (password != this->_serverPass)
		return ":" + this->_serverName + " 464 " + client.getReplyNick() + " :Password incorrect\r\n";
	client.setAuth(true);
	return "";		// Here we need to see if this type of response that should just be ignored can mess with anything else or not
}

std::string	Server::handleNick(Client& client, std::istringstream& in) {
	std::string nickname;
	in >> nickname;

	if (nickname.empty())
		return ":" + this->_serverName + " 461 " + client.getReplyNick() + " NICK: Not enough parameters\r\n";
	for (auto it = _clients.begin(); it != _clients.end(); ++it){
		if (it->second.getNick() == nickname)
			return ":" + this->_serverName + " 433 * " + nickname + " :Nickname is already taken\r\n";
	}
	client.setNick(nickname);
	return "";		// Here the same story, stay updated for the next episode
}

std::string	Server::handleUser(Client& client, std::istringstream& in) {
	
}

std::string	Server::handleJoin(Client& client, std::istringstream& in) {return "";}

std::string	Server::handlePrivmsg(Client& client, std::istringstream& in) {return "";}

std::string	Server::handlePing(Client& client, std::istringstream& in) {return "";}

std::string	Server::handlePong(Client& client, std::istringstream& in) {return "";}

std::string	Server::handlePart(Client& client, std::istringstream& in) {return "";}

std::string	Server::handleQuit(Client& client, std::istringstream& in) {return "";}

std::string	Server::handleKick(Client& client, std::istringstream& in) {return "";}

std::string	Server::handleInvite(Client& client, std::istringstream& in) {return "";}

std::string	Server::handleTopic(Client& client, std::istringstream& in) {return "";}

std::string	Server::handleMode(Client& client, std::istringstream& in) {return "";}
