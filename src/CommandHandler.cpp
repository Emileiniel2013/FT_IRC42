/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CommandHandler.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: temil-da <temil-da@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/26 14:33:56 by temil-da          #+#    #+#             */
/*   Updated: 2025/09/26 14:49:43 by temil-da         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "ErrorCodes.hpp"
#include <sstream>
#include <vector>
#include <sys/socket.h>
#include "ServerHelpers.hpp"


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
		_clients.erase(client.getId());
		return ;
	}
	client.setPassOk(true);
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
	client.setNickOk(true);
	registerUser(client);
}

void	Server::handleUser(Client& client, std::istringstream& in) {
	std::string	username, hostname, servername, realname;
	in >> username >> hostname >> servername;
	std::getline(in, realname);
	trimPrefix(realname);
	if (username.empty() || hostname.empty() || servername.empty() || realname.empty()){
		sendError(client, ERR_NEEDMOREPARAMS, "USER", "Not enough parameters");
		return ;
	}
	if (client.getUser() != ""){
		sendError(client, ERR_ALREADYREGISTRED, "", "You might not reregister");
		return ;
	}
	client.setUser(username);
	client.setName(realname);
	client.setUserOk(true);
	registerUser(client);
}

void	Server::handleJoin(Client& client, std::istringstream& in) {
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
		if (chName.empty() || chName[0] != '#'){
			sendError(client, ERR_NOSUCHCHANNEL, chName, "No such channel");
			continue;
		}
		std::string	key;
		if (i < passwords.size()){
			key = passwords[i];
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
	std::string	target;
	in >> target;
	std::string message;
	std::getline(in, message);
	trimPrefix(message);
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
		std::string	msg = ":" + client.getPrefix() + 
			" PRIVMSG " + target + " :" + message + "\r\n";
		send(getIdFromNick(target), msg.c_str(), msg.size(), 0);
	}
}

void	Server::handlePing(Client& client, std::istringstream& in) {
	std::string	token;
	std::getline(in, token);
	trimPrefix(token);
	if (token.empty()){
		sendError(client, ERR_NOORIGIN, "PING", "No origin specified");
	}
	std::string	msg = ":" + this->_serverName + " PONG :" + token + "\r\n";
	send(client.getId(), msg.c_str(), msg.size(), 0);
}

void	Server::handlePart(Client& client, std::istringstream& in) {
	std::string	chStr, reason;
	in >> chStr;
	if (chStr.empty()){
		sendError(client, ERR_NEEDMOREPARAMS, "PART", "Not enough parameters");
		return ;
	}
	std::getline(in, reason);
	trimPrefix(reason);
	if (reason.empty())
		reason = client.getNick();
	std::vector<std::string>	channels = split(chStr, ',');
	for (size_t i = 0; i < channels.size(); ++i){
		std::string chName = channels[i];
		if (_channels.count(chName) == 0){
			sendError(client, ERR_NOSUCHCHANNEL, chName, "No such channel");
			continue ;
		}
		Channel& ch = _channels[chName];
		if (!ch.isMember(client.getId())){
			sendError(client, ERR_NOTONCHANNEL, chName, "You're not on that channel");
			continue ;
		}
		std::string msg = ":" + client.getPrefix() + " PART " + chName + " :" + 
			reason + "\r\n";
		ch.removeMember(client.getId());
		ch.removeOperator(client.getId());
		ch.broadcast(msg);
		client.remChannel(chName);
		if (ch.getAllMembers().empty())
			_channels.erase(chName);
	}
}

void	Server::handleQuit(Client& client, std::istringstream& in) {
	std::string	reason;
	std::getline(in, reason);
	trimPrefix(reason);
	if (reason.empty())
		reason = client.getNick();
	std::string	msg = ":" + client.getPrefix() + _serverName + " QUIT :" + reason + "\r\n";
	std::set<std::string>	channels = client.getChannels();
	for (auto it = channels.begin(); it != channels.end(); ++it){
		std::string	chName = *it;
		if (_channels.count(chName) == 0)
			continue ;
		Channel&	ch = _channels[chName];
		ch.removeMember(client.getId());
		ch.removeOperator(client.getId());
		ch.broadcast(msg);
		if (ch.getAllMembers().empty())
			_channels.erase(chName);
	}
	int fd_to_close = client.getId(); // HERE WE NEED TO SEND THE FD TO THE SERVER SYSTEM THAT CLOSES THE FDS
	_clients.erase(fd_to_close);
}

void	Server::handleKick(Client& client, std::istringstream& in) {
	std::string	chName, target, res;
	in >> chName >> target;
	std::getline(in, res);
	if (chName.empty() || target.empty()){
		sendError(client, ERR_NEEDMOREPARAMS, "KICK", "Not enough parameters");
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
	if (!isOnServer(target)) {
		sendError(client, ERR_NOSUCHNICK, target + " " + ch.getName(), "No such nick");
		return ;
	}
	int	targetId = getIdFromNick(target);
	if (!ch.isMember(targetId)){
		sendError(client, ERR_USERNOTINCHANNEL, target + " " + ch.getName(), "They aren't on that channel");
		return ;
	}
	trimPrefix(res);
	if (res.empty())
		res = client.getNick();
	std::string	msg = ":" + client.getPrefix() + " KICK " + chName + " " + target 
		+ " :" + res + "\r\n";
	ch.broadcast(msg);
	ch.removeMember(targetId);
	ch.removeOperator(targetId);
	_clients[targetId].remChannel(chName);
}

void	Server::handleInvite(Client& client, std::istringstream& in) {
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
	std::string	chName, topic;
	in >> chName;
	if (chName.empty()){
		sendError(client, ERR_NEEDMOREPARAMS, "TOPIC", "Not enough parameters");
		return ;
	}
	if (_channels.count(chName) == 0){
		sendError(client, ERR_NOSUCHCHANNEL, chName, "No such channel");
		return ;
	}
	std::getline(in, topic);
	trimPrefix(topic);
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
	std::string	msg = ":" + client.getPrefix() + 
		" TOPIC " + chName + " :" + topic + "\r\n";
	ch.broadcast(msg);
}

void	Server::handleMode(Client& client, std::istringstream& in) {
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
		{
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
		}
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
	std::string	msg = ":" + client.getPrefix() + " MODE " + chName + " " + mode;
	if (!param.empty())
		msg += " " + param;
	msg += "\r\n";
	ch.broadcast(msg);
}
