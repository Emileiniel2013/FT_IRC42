/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: temil-da <temil-da@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/15 17:02:14 by temil-da          #+#    #+#             */
/*   Updated: 2025/09/19 19:45:44 by temil-da         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Utils.hpp"
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

void		Server::registerUser(Client& client){
	if (client.getAuth())
		return ;
	if (!client.getPassOk())
		return ;
	if (!client.getNickOk() || !client.getUserOk())
		return ;
	client.setAuth(true);
	std::string	msg = ":" + _serverName + " 001 " + client.getNick() + " :WELCOME TO THE GREEN AVENGERS IRC NETWORK, "
		+ client.getNick() + "!" + client.getUser() + "@" + _serverName + "\r\n";
	send(client.getId(), msg.c_str(), msg.size(), 0);
}


void	Server::processInput(Client& client, std::string& buf) 
{
	std::string			cmd;
	std::istringstream	keyword(buf);

	keyword >> cmd;
	auto	it = this->_cmdMap.find(cmd);
	if (it != this->_cmdMap.end())
		(this->*(it->second))(client, keyword);
	else{
		std::string msg = ":" + this->_serverName + " 421 " + client.getReplyNick() + " " + cmd + " :Unknown command\r\n";
		send(client.getId(), msg.c_str(), msg.size(), 0);
	}
}

void	Server::handlePass(Client& client, std::istringstream& in) {
	std::string	password;
	in >> password;
	std::string	msg;
	
	if (password.empty()){
		msg = ":" + this->_serverName + " 461 " + client.getReplyNick() + " PASS: Not enough parameters\r\n";
		send(client.getId(), msg.c_str(), msg.size(), 0);
		return ;
	}
	if (client.getAuth()){
		msg = ":" + this->_serverName + " 462 " + client.getReplyNick() + " :You might not reregister\r\n";
		send(client.getId(), msg.c_str(), msg.size(), 0);
		return ;
	}
	if (password != this->_serverPass){
		msg = ":" + this->_serverName + " 464 " + client.getReplyNick() + " :Password incorrect\r\n"; //HERE WE NEED TO DISCONNECT USER
		send(client.getId(), msg.c_str(), msg.size(), 0);
		return ;
	}
	registerUser(client);
}

void	Server::handleNick(Client& client, std::istringstream& in) {
	std::string nickname;
	in >> nickname;
	std::string	msg;

	if (nickname.empty()){
		msg = ":" + this->_serverName + " 461 " + client.getReplyNick() + " NICK: Not enough parameters\r\n";
		send(client.getId(), msg.c_str(), msg.size(), 0);
		return ;
	}
	for (auto it = _clients.begin(); it != _clients.end(); ++it){
		if (it->second.getNick() == nickname){
			msg = ":" + this->_serverName + " 433 * " + nickname + " :Nickname is already taken\r\n";
			send(client.getId(), msg.c_str(), msg.size(), 0);
			return ;
		}
	}
	client.setNick(nickname);
	registerUser(client);
}

void	Server::handleUser(Client& client, std::istringstream& in) {
	std::string	username, hostname, servername, realname;
	in >> username >> hostname >> servername;
	std::getline(in, realname);
	std::string	msg;
	if (username.empty() || hostname.empty() || servername.empty() || realname.empty()){
		msg = ":" + this->_serverName + " 461 " + client.getReplyNick() + " USER: Not enough parameters\r\n";
		send(client.getId(), msg.c_str(), msg.size(), 0);
		return ;
	}
	if (client.getUser() != ""){
		msg = ":" + this->_serverName + " 462 " + client.getReplyNick() + " :You might not reregister\r\n";
		send(client.getId(), msg.c_str(), msg.size(), 0);
		return ;
	}
	realname.erase(0, 1);
	client.setUser(username);
	client.setName(realname);
	registerUser(client);
}

void	Server::handleJoin(Client& client, std::istringstream& in) {
	std::string	msg;
	if (!client.getAuth()){
		msg = ":" + this->_serverName + " 451 " + client.getReplyNick() + " :You have not registered\r\n";
		send(client.getId(), msg.c_str(), msg.size(), 0);
		return ;
	}

	std::string	chStr;
	std::string	paswdStr;
	in >> chStr;
	if (chStr.empty()){
		msg = ":" + this->_serverName + " 461 " + client.getReplyNick() + " JOIN: Not enough parameters\r\n";
		send(client.getId(), msg.c_str(), msg.size(), 0);
		return ;
	}
	in >> paswdStr;

	std::vector<std::string>	channels = split(chStr, ',');
	std::vector<std::string>	passwords = split(paswdStr, ',');
	for (size_t i = 0; i < channels.size(); ++i){
		std::string chName = channels[i];
		if (_channels.count(chName) == 0){
			_channels[chName] = Channel(chName);
			_channels[chName].addOperator(client.getId());
			client.addChannel(chName);
			continue;
		}
		Channel& ch = _channels[chName];
		if (!ch.getPass().empty() && (passwords.empty() || ch.getPass() != passwords.front())){
			msg = ":" + this->_serverName + " 475 " + client.getReplyNick() + " " + chName + " :Cannot join channel (+k)\r\n";
			send(client.getId(), msg.c_str(), msg.size(), 0);
			if (!passwords.empty())
				passwords.erase(passwords.begin());
			continue;
		}
		//NEXT PART INVITEONLY CHECK, I JUST IMPLEMENTED AN INVITATION LIST TO USE HERE
	}
}

void	Server::handlePrivmsg(Client& client, std::istringstream& in) {}

void	Server::handlePing(Client& client, std::istringstream& in) {}

void	Server::handlePong(Client& client, std::istringstream& in) {}

void	Server::handlePart(Client& client, std::istringstream& in) {}

void	Server::handleQuit(Client& client, std::istringstream& in) {}

void	Server::handleKick(Client& client, std::istringstream& in) {}

void	Server::handleInvite(Client& client, std::istringstream& in) {}

void	Server::handleTopic(Client& client, std::istringstream& in) {}

void	Server::handleMode(Client& client, std::istringstream& in) {}
