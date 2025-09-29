/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: temil-da <temil-da@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/15 14:57:21 by temil-da          #+#    #+#             */
/*   Updated: 2025/09/29 19:51:34 by temil-da         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include <time.h>

Client::Client() : _id(-1) {}

Client::Client(int id) : _id(id) {}

Client::Client(const Client& other) {
	_id = other._id;
	_nick = other._nick;
	_user = other._user;
	_name = other._name;
	_hostname = other._hostname;
	_channels = other._channels;
	_authenticated = other._authenticated;
	_inputBuf = other._inputBuf;
}

Client&	Client::operator=(const Client& other){
	if (this != &other){
		_id = other._id;
		_nick = other._nick;
		_user = other._user;
		_name = other._name;
		_hostname = other._hostname;
		_channels = other._channels;
		_authenticated = other._authenticated;
		_inputBuf = other._inputBuf;
	}
	return *this;
}

Client::~Client() {}

//GETTERS/SETTERS
int						Client::getId() const {return this->_id;}
void					Client::setId(int id) {this->_id = id;}

const	std::string&	Client::getNick() const {return this->_nick;}
std::string				Client::getReplyNick() const {
	if (!this->_nick.empty())
		return this->_nick;
	return "*";
}

std::string				Client::getPrefix() const {
	return getReplyNick() + "!" + getUser() + "@" + _hostname;
}

void					Client::setNick(const std::string& nick) {this->_nick = nick;}

void					Client::setUser(const std::string& user) {this->_user = user;}
const 	std::string&	Client::getUser() const {return this->_user;}

const std::string&		Client::getName() const {return this->_name;}
void					Client::setName(const std::string& name) {this->_name = name;}

void					Client::setHostname(const std::string& host) {_hostname = host;}

std::set<std::string>	Client::getChannels() const {return this->_channels;}
void					Client::addChannel(const std::string& channel) {this->_channels.insert(channel);}
void					Client::remChannel(const std::string& channel){this->_channels.erase(channel);}

bool					Client::getPassOk() const {return this->_passOk;}
void					Client::setPassOk(bool b) {this->_passOk = b;}
bool					Client::getNickOk() const {return this->_nickOk;}
void					Client::setNickOk(bool b) {this->_nickOk = b;}
void					Client::setUserOk(bool b) {this->_userOk = b;}
bool					Client::getUserOk() const {return this->_userOk;}
bool					Client::getAuth() const {return this->_authenticated;}
void					Client::setAuth(bool b){
	this->_authenticated = b;
}

void					Client::setExit(bool b) {this->_exit = b;}
bool					Client::getExit(void) const {return this->_exit;}

std::string&			Client::getInputBuf() {return this->_inputBuf;}

void					Client::appendInputBuf(const std::string& data) {
	this->_inputBuf += data;
}

void					Client::eraseFromInputBuf(size_t count) {
	this->_inputBuf.erase(0, count);
}
