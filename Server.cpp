/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: temil-da <temil-da@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/15 17:02:14 by temil-da          #+#    #+#             */
/*   Updated: 2025/09/15 19:07:18 by temil-da         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Commands.hpp"
#include <sstream>

Server::Server(std::string name, std::string pass) : _serverName(name), _serverPass(pass) {
	initCmdTable(_cmdMap);
}

Server::Server(const Server& other) : _serverName(other._serverName), _serverPass(other._serverPass), _cmdMap(other._cmdMap) {}

Server&	Server::operator=(const Server& other) {
	if (this != &other){
		_serverName = other._serverName;
		_serverPass = other._serverPass;
		_cmdMap = other._cmdMap;
	}
	return *this;
}

Server::~Server() {}

const std::string&	Server::getName() {return this->_serverName;}
const std::string&	Server::getPass() {return this->_serverPass;}

std::string	Server::processInput(Client& client, std::map<std::string, Channel>& channels,
							std::map<int, Client>& clients,
							std::string& buf) 
{
	std::string			cmd;
	std::istringstream	keyword(buf);

	keyword >> cmd;
	auto	it = this->_cmdMap.find(cmd);
	if (it != this->_cmdMap.end())
		return it->second(client, channels, clients, keyword);
	else
		return ":" + this->_serverName + " 421 " + client.getNick() + " " + cmd + " :Unknown command\r\n";
}
