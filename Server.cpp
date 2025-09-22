/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: temil-da <temil-da@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/15 17:02:14 by temil-da          #+#    #+#             */
/*   Updated: 2025/09/22 17:17:06 by temil-da         ###   ########.fr       */
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
	if (it != this->_cmdMap.end())
		(this->*(it->second))(client, keyword);
	else
		sendError(client, ERR_UNKNOWNCOMMAND, cmd, "Unknown command");
}

void	Server::handlePass(Client& client, std::istringstream& in) {
	std::string	password;
	in >> password;
	
	if (password.empty()){
		sendError(client, ERR_NEEDMOREPARAMS, "PASS", "Not enough parameters");
		return ;
	}
	if (client.getAuth()){
		sendError(client, ERR_ALREADYREGISTRED, "", "You might not reregister");
		return ;
	}
	if (password != this->_serverPass){
		sendError(client, ERR_PASSWDMISMATCH, "", "Password incorrect"); //HERE WE NEED TO DISCONNECT USER
		return ;
	}
	registerUser(client);
}

void	Server::handleNick(Client& client, std::istringstream& in) {
	std::string nickname;
	in >> nickname;

	if (nickname.empty()){
		sendError(client, ERR_NEEDMOREPARAMS, "NICK", "Not enough parameters");
		return ;
	}
	if (isOnServer(nickname)){
		sendError(client, ERR_NICKNAMEINUSE, nickname, "Nickname is already taken");
		return ;
	}
	client.setNick(nickname);
	registerUser(client);
}

void	Server::handleUser(Client& client, std::istringstream& in) {
	std::string	username, hostname, servername, realname;
	in >> username >> hostname >> servername;
	std::getline(in, realname);
	if (username.empty() || hostname.empty() || servername.empty() || realname.empty()){
		sendError(client, ERR_NEEDMOREPARAMS, "USER", "Not enough parameters");
		return ;
	}
	if (client.getUser() != ""){
		sendError(client, ERR_ALREADYREGISTRED, "", "You might not reregister");
		return ;
	}
	realname.erase(0, 1);
	client.setUser(username);
	client.setName(realname);
	registerUser(client);
}

void	Server::handleJoin(Client& client, std::istringstream& in) {
	if (!client.getAuth()){
		sendError(client, ERR_NOTREGISTERED, "", "You have not registered");
		return ;
	}
	std::string	chStr;
	std::string	paswdStr;
	in >> chStr;
	if (chStr.empty()){
		sendError(client, ERR_NEEDMOREPARAMS, "JOIN", "Not enough parameters");
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
			createChannel(client, chName);
			continue;
		}
		Channel& ch = _channels[chName];
		if (!ch.getPass().empty() && (key.empty() || ch.getPass() != key)){
			sendError(client, ERR_BADCHANNELKEY, chName, "Cannot join channel (+k)");
			continue;
		}
		if (ch.getInviteOnly() && !ch.isInvited(client.getId())){
			sendError(client, ERR_INVITEONLYCHAN, chName, "Cannot join channel (+i)");
			continue;
		}
		if (ch.getUserLimit() != 0 && ch.getUserCount() >= ch.getUserLimit()){
			sendError(client, ERR_CHANNELISFULL, chName, "Cannot join channel (+l)");
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
	if (!client.getAuth()){
		sendError(client, ERR_NOTREGISTERED, "", "You have not registered");
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
		sendError(client, ERR_NORECIPIENT, "", "No recipient given (PRIVMSG)");
		return ;
	}
	if (message.empty()){
		sendError(client, ERR_NOTEXTTOSEND, "", "No text to send");
		return ;
	}
	if (target[0] == '#'){
		if (_channels.count(target) == 0){
			sendError(client, ERR_NOSUCHCHANNEL, target, "No such channel");
			return ;
		}
		if (!_channels[target].isMember(client.getId())){
			sendError(client, ERR_NOTONCHANNEL, target, "You're not on that channel");
			return ;
		}
		broadcastMessage(client, _channels[target], message);
	}else {
		if (!isOnServer(target)){
			sendError(client, ERR_NOSUCHNICK, target, "No such nick");
			return ;
		}
		std::string	msg = ":" + client.getPrefix() + this->_serverName + 
			" PRIVMSG " + target + " :" + message + "\r\n";
		send(getIdFromNick(target), msg.c_str(), msg.size(), 0);
		send(client.getId(), msg.c_str(), msg.size(), 0);
	}
}

void	Server::handlePing(Client& client, std::istringstream& in) {
	if (!client.getAuth()){
		sendError(client, ERR_NOTREGISTERED, "", "You have not registered");
		return ;
	}
	std::string	token;
	std::getline(in, token);
	if (!token.empty() || token[0] == ' ')
		token.erase(0, 1);
	if (!token.empty() || token[0] == ':')
		token.erase(0, 1);
	if (token.empty()){
		sendError(client, ERR_NOORIGIN, "PING", "No origin specified");
	}
	std::string	msg = ":" + this->_serverName + " PONG :" + token + "\r\n";
	send(client.getId(), msg.c_str(), msg.size(), 0);
}

void	Server::handlePart(Client& client, std::istringstream& in) {}

void	Server::handleQuit(Client& client, std::istringstream& in) {}

void	Server::handleKick(Client& client, std::istringstream& in) {
	
}

void	Server::handleInvite(Client& client, std::istringstream& in) {
	if (!client.getAuth()){
		sendError(client, ERR_NOTREGISTERED, "", "You have not registered");
		return ;
	}
	std::string	targetName, chName;
	in >> targetName >> chName;
	if (targetName.empty() || chName.empty()){
		sendError(client, ERR_NEEDMOREPARAMS, "INVITE", "Not enough parameters");
		return ;
	}
	if (_channels.count(chName) == 0){
		sendError(client, ERR_NOSUCHCHANNEL, chName, "No such channel");
		return ;
	}
	Channel&	ch = _channels[chName];
	if (!ch.isOperator(client.getId())){
		sendError(client, ERR_CHANOPRIVSNEEDED, chName, "You're not channel operator");
		return ;
	}
	if (!isOnServer(targetName)){
		sendError(client, ERR_NOSUCHNICK, targetName, "No such nick");
		return ;		
	}
	if (ch.getInviteOnly())
		ch.addInvite(getIdFromNick(targetName));
	std::string	msg = ":" + client.getPrefix() + this->_serverName + 
		" INVITE " + targetName + " :" + chName + "\r\n";
	send(getIdFromNick(targetName), msg.c_str(), msg.size(), 0);
}

void	Server::handleTopic(Client& client, std::istringstream& in) {
	if (!client.getAuth()){
		sendError(client, ERR_NOTREGISTERED, "", "You have not registered");
		return ;
	}
	std::string	chName, topic;
	in >> chName;
	if (chName.empty()){
		sendError(client, ERR_NEEDMOREPARAMS, "TOPIC", "Not enough parameters");
		return ;
	}ERR_NOTREGISTERED
	if (_channels.count(chName) == 0){
		sendError(client, ERR_NOSUCHCHANNEL, chName, "No such channel");
		return ;
	}
	std::getline(in, topic);
	if (!topic.empty() && topic[0] == ' ')
		topic.erase(0, 1);
	if (!topic.empty() && topic[0] == ':')
		topic.erase(0, 1);
	Channel&	ch = _channels[chName];
	if (!ch.isMember(client.getId())){
		sendError(client, ERR_NOTONCHANNEL, chName, "You're not on that channel");
		return ;
	}
	if (topic.empty()){
		std::string	msg;
		if (ch.getTopic().empty()){
			msg = ":" + this->_serverName + " 331 " + client.getReplyNick() + " " 
				+ chName + " :No topic is set\r\n";
			send(client.getId(), msg.c_str(), msg.size(), 0);
		}else {
			msg = ":" + this->_serverName + " 332 " + client.getReplyNick() + " " 
				+ chName + " :" + ch.getTopic() + "\r\n";
			send(client.getId(), msg.c_str(), msg.size(), 0);
		}
		return ;
	}
	if (ch.getTopicRestrict() && !ch.isOperator(client.getId())){
		sendError(client, ERR_CHANOPRIVSNEEDED, chName, "You're not channel operator");
		return ;
	}
	ch.setTopic(topic);
	std::string	msg = ":" + client.getPrefix() + _serverName + 
		" TOPIC " + chName + " :" + topic + "\r\n";
	for (int id : ch.getAllMembers())
		send(id, msg.c_str(), msg.size(), 0);
}

void	Server::handleMode(Client& client, std::istringstream& in) {
	if (!client.getAuth()){
		sendError(client, ERR_NOTREGISTERED, "", "You have not registered");
		return ;
	}
	std::string	chName, mode, param;
	in >> chName >> mode >> param;
	if (chName.empty() || mode.empty()){
		sendError(client, ERR_NEEDMOREPARAMS, "MODE", "Not enough parameters");
		return ;
	}
	if (_channels.count(chName) == 0){
		sendError(client, ERR_NOSUCHCHANNEL, chName, "No such channel");
		return ;
	}
	Channel&	ch = _channels[chName];
	if (!ch.isMember(client.getId())){
		sendError(client, ERR_NOTONCHANNEL, chName, "You're not on that channel");
		return ;
	}
	if (!ch.isOperator(client.getId())){
		sendError(client, ERR_CHANOPRIVSNEEDED, chName, "You're not channel operator");
		return ;
	}
	char	sign = mode[0];
	if ((sign != '+' && sign != '-') || mode.size() != 2){
		sendError(client, ERR_UNKNOWNMODE, mode, "Unknown mode char");
		return ;
	}
	char	cmd = mode[1];
	switch (cmd) {
		case 'i':
			ch.setInviteOnly(sign == '+');
			break ;
		case 't':
			ch.setTopicRestrict(sign == '+');
			break ;
		case 'k':
			if (sign == '+') {
				if (param.empty()){
					sendError(client, ERR_NEEDMOREPARAMS, "MODE", "Missing key parameter");
					return ;
				}
				ch.setPass(param);
			}
			else
				ch.setPass("");
			break ;
		case 'o':
			if (param.empty()){
					sendError(client, ERR_NEEDMOREPARAMS, "MODE", "Missing nick parameter");
					return ;				
			}
			if (!isOnServer(param)) {
				sendError(client, ERR_NOSUCHNICK, param + " " + ch.getName(), "No such nick");
				return ;
			}
			if (!isInChannel(param, ch)) {
				sendError(client, ERR_USERNOTINCHANNEL, param + " " + ch.getName(), "They aren't on that channel");
				return ;
			}
			int id = getIdFromNick(param);
			if (sign == '+'){
				ch.addOperator(id);
				ch.removeMember(id);
			}else {
				ch.removeOperator(id);
				ch.addMember(id);
			}
			break ;
		case 'l':
			if (sign == '+'){
				if (param.empty()){
					sendError(client, ERR_NEEDMOREPARAMS, "MODE", "Missing limit parameter");
					return ;
				}
				try {
					int	limit = std::stoi(param);
					if (limit <= 0)
						throw std::invalid_argument("negative");
					ch.setUserLimit(limit);
				} catch	(std::exception& e) {
					sendError(client, ERR_UNKNOWNMODE, param, "Invalid mode parameter");
					return ;
				}
			}
			else
				ch.setUserLimit(0);
			break ;
		default:
			sendError(client, ERR_UNKNOWNMODE, std::string(1, cmd), "Unknown mode char");
			return ;
	}
	std::string	msg = ":" + client.getPrefix() + _serverName + " MODE " + chName + " " + mode;
	if (!param.empty())
		msg += " " + param;
	for (int id : ch.getAllMembers())
		send(id, msg.c_str(), msg.size(), 0);
}
