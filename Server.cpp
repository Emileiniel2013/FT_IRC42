/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: temil-da <temil-da@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/15 17:02:14 by temil-da          #+#    #+#             */
/*   Updated: 2025/09/21 15:50:17 by temil-da         ###   ########.fr       */
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

void	Server::broadcastJoin(Client& client, Channel& ch) {
	std::string	msg = ":" + client.getNick() + "!~" + client.getUser() + "@" + this->_serverName +
		" JOIN :" + ch.getName() + "\r\n";
	for (int id : ch.getAllMembers()){
		send(id, msg.c_str(), msg.size(), 0);
	}
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
}

void	Server::broadcastMessage(Client& sender, Channel& ch, const std::string message){
	std::string	msg = ":" + sender.getNick() + "!~" + sender.getUser() + "@" + this->_serverName + 
			" PRIVMSG " + ch.getName() + " :" + message + "\r\n";
	for (int id : ch.getAllMembers())
		send(id, msg.c_str(), msg.size(), 0);
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
		msg = ":" + this->_serverName + " 461 " + client.getReplyNick() + " NICK: Not enough parameter && !_channels[chName].isOperator(client.getId())s\r\n";
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
		std::string	key;
		if (!passwords.empty()){
			key = passwords.front();
			passwords.erase(passwords.begin());
		}
		if (_channels.count(chName) == 0){
			_channels[chName] = Channel(chName);
			_channels[chName].addOperator(client.getId());
			client.addChannel(chName);
			sendTopic(client, _channels[chName]);
			sendNames(client, _channels[chName]);
			continue;
		}
		Channel& ch = _channels[chName];
		if (!ch.getPass().empty() && (key.empty() || ch.getPass() != key)){
			msg = ":" + this->_serverName + " 475 " + client.getReplyNick() + " " + chName + " :Cannot join channel (+k)\r\n";
			send(client.getId(), msg.c_str(), msg.size(), 0);
			continue;
		}
		if (ch.getInviteOnly() && !ch.isInvited(client.getId())){
			msg = ":" + this->_serverName + " 473 " + client.getReplyNick() + " " + chName + " :Cannot join channel (+i)\r\n";
			send(client.getId(), msg.c_str(), msg.size(), 0);
			continue;
		}
		if (ch.getUserLimit() != 0 && ch.getUserCount() >= ch.getUserLimit()){
			msg = ":" + this->_serverName + " 471 " + client.getReplyNick() + " " + chName + " :Cannot join channel (+l)\r\n";
			send(client.getId(), msg.c_str(), msg.size(), 0);
			continue;
		}
		ch.addMember(client.getId());
		client.addChannel(chName);
		broadcastJoin(client, ch);
		sendTopic(client, ch);
		sendNames(client, ch);
	}
}

void	Server::handlePrivmsg(Client& client, std::istringstream& in) {
	std::string	msg;
	if (!client.getAuth()){
		msg = ":" + this->_serverName + " 451 " + client.getReplyNick() + " :You have not registered\r\n";
		send(client.getId(), msg.c_str(), msg.size(), 0);
		return ;
	}
	std::string	target;
	in >> target;
	std::string message;
	std::getline(in, message);
	if (!message.empty()){
		if (message[0] == ' ')
			message.erase(0, 1);
		if (message[0] == ':')
			message.erase(0, 1);
	}
	if (target.empty() || target[0] == ':'){
		msg = ":" + this->_serverName + " 411 " + client.getReplyNick() + " :No recipient given (PRIVMSG)\r\n";
		send(client.getId(), msg.c_str(), msg.size(), 0);
		return ;
	}
	if (message.empty()){
		msg = ":" + this->_serverName + " 412 " + client.getReplyNick() + " :No text to send\r\n";
		send(client.getId(), msg.c_str(), msg.size(), 0);
		return ;
	}
	if (target[0] == '#'){
		if (_channels.count(target) == 0){
			msg = ":" + this->_serverName + " 403 " + client.getReplyNick() + " " + target + " :No such channel\r\n";
			send(client.getId(), msg.c_str(), msg.size(), 0);
			return ;
		}
		if (!_channels[target].isMember(client.getId())){
			msg = ":" + this->_serverName + " 442 " + client.getReplyNick() + " " + target + " :You're not on that channel\r\n";
			send(client.getId(), msg.c_str(), msg.size(), 0);
			return ;
		}
		broadcastMessage(client, _channels[target], message);
	}else {
		Client*	recipient = nullptr;
		for (auto it = _clients.begin(); it != _clients.end(); ++it){
			if (it->second.getNick() == target){
				recipient = &it->second;
				break ;
			}
		}
		if (!recipient){
			msg = ":" + this->_serverName + " 401 " + client.getReplyNick() + " " + target + " :No such nick\r\n";
			send(client.getId(), msg.c_str(), msg.size(), 0);
			return ;
		}
		msg = ":" + client.getNick() + "!~" + client.getUser() + "@" + this->_serverName + 
			" PRIVMSG " + target + " :" + message + "\r\n";
		send(recipient->getId(), msg.c_str(), msg.size(), 0);
		send(client.getId(), msg.c_str(), msg.size(), 0);
	}
}

void	Server::handlePing(Client& client, std::istringstream& in) {}

void	Server::handlePong(Client& client, std::istringstream& in) {}

void	Server::handlePart(Client& client, std::istringstream& in) {}

void	Server::handleQuit(Client& client, std::istringstream& in) {}

void	Server::handleKick(Client& client, std::istringstream& in) {}

void	Server::handleInvite(Client& client, std::istringstream& in) {
	std::string	msg;
	if (!client.getAuth()){
		msg = ":" + this->_serverName + " 451 " + client.getReplyNick() 
			+ " :You have not registered\r\n";
		send(client.getId(), msg.c_str(), msg.size(), 0);
		return ;
	}
	std::string	targetName, chName;
	in >> targetName >> chName;
	if (targetName.empty() || chName.empty()){
		msg = ":" + this->_serverName + " 461 " + client.getReplyNick() 
			+ " INVITE:Not enough parameters\r\n";
		send(client.getId(), msg.c_str(), msg.size(), 0);
		return ;
	}
	if (_channels.count(chName) == 0){
		msg = ":" + this->_serverName + " 403 " + client.getReplyNick() + " " 
			+ chName + " :No such channel\r\n";
		send(client.getId(), msg.c_str(), msg.size(), 0);
		return ;
	}
	Channel&	ch = _channels[chName];
	if (!ch.isOperator(client.getId())){
		msg = ":" + this->_serverName + " 482 " + client.getReplyNick() + " " 
			+ chName + " :You're not channel operator\r\n";
		send(client.getId(), msg.c_str(), msg.size(), 0);
		return ;
	}
	Client*	target = nullptr;
	for (auto it = _clients.begin(); it != _clients.end(); ++it){
		if (it->second.getNick() == targetName){
			target = &it->second;
			break ;
		}
	}
	if (!target){
		msg = ":" + this->_serverName + " 401 " + client.getReplyNick() + " " + 
			targetName + " :No such nick\r\n";
		send(client.getId(), msg.c_str(), msg.size(), 0);
		return ;		
	}
	if (ch.getInviteOnly())
		ch.addInvite(target->getId());
	msg = ":" + client.getNick() + "!~" + client.getUser() + "@" + this->_serverName + 
		" INVITE " + targetName + " :" + chName + "\r\n";
	send(target->getId(), msg.c_str(), msg.size(), 0);
}

void	Server::handleTopic(Client& client, std::istringstream& in) {
	std::string	msg;
	if (!client.getAuth()){
		msg = ":" + this->_serverName + " 451 " + client.getReplyNick() 
			+ " :You have not registered\r\n";
		send(client.getId(), msg.c_str(), msg.size(), 0);
		return ;
	}
	std::string	chName, topic;
	in >> chName >> topic;
	if (chName.empty()){
		msg = ":" + this->_serverName + " 461 " + client.getReplyNick() 
			+ " TOPIC:Not enough parameters\r\n";
		send(client.getId(), msg.c_str(), msg.size(), 0);
		return ;
	}
	if (_channels.count(chName) == 0){
		msg = ":" + this->_serverName + " 403 " + client.getReplyNick() + " " 
			+ chName + " :No such channel\r\n";
		send(client.getId(), msg.c_str(), msg.size(), 0);
		return ;
	}
	if (!_channels[chName].isMember(client.getId())){
		msg = ":" + this->_serverName + " 442 " + client.getReplyNick() + " " 
			+ chName + " :You're not on that channel\r\n";
		send(client.getId(), msg.c_str(), msg.size(), 0);
		return ;
	}
	if (topic.empty()){
		if (_channels[chName].getTopic().empty()){
			msg = ":" + this->_serverName + " 331 " + client.getReplyNick() + " " 
				+ chName + " :No topic is set\r\n";
			send(client.getId(), msg.c_str(), msg.size(), 0);
			return ;
		}
		msg = ":" + this->_serverName + " 332 " + client.getReplyNick() + " " 
			+ chName + " :" + _channels[chName].getTopic() + "\r\n";
		send(client.getId(), msg.c_str(), msg.size(), 0);
		return ;
	}
}

void	Server::handleMode(Client& client, std::istringstream& in) {}
