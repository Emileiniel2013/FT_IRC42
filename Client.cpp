/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: temil-da <temil-da@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/15 14:57:21 by temil-da          #+#    #+#             */
/*   Updated: 2025/09/17 17:57:48 by temil-da         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client(int id) : _id(id) {}

Client::Client(const Client& other) {
	_id = other._id;
	_nick = other._nick;
	_user = other._user;
	_name = other._name;
	_channels = other._channels;
	_auth = other._auth;
	_lastInput = other._lastInput;
	_ping = other._ping;
	_inputBuf = other._inputBuf;
}

Client&	Client::operator=(const Client& other){
	if (this != &other){
		_id = other._id;
		_nick = other._nick;
		_user = other._user;
		_name = other._name;
		_channels = other._channels;
		_auth = other._auth;
		_lastInput = other._lastInput;
		_ping = other._ping;
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

void					Client::setNick(const std::string& nick) {this->_nick = nick;}

void					Client::setUser(const std::string& user) {this->_user = user;}
const 	std::string&	Client::getUser() const {return this->_user;}

bool					Client::getAuth() const {return this->_auth;}
void					Client::setAuth(bool b){
	this->_auth = b;
}


bool					Client::isInChannel(const std::string& channel) const {
	return this->_channels.find(channel) != this->_channels.end();
}

void					Client::addChannel(const std::string& channel) {
	this->_channels.insert(channel);
}

void					Client::removeChannel(const std::string& channel){
	this->_channels.erase(channel);
}

time_t					Client::getLastInput() const {return this->_lastInput;}
void					Client::setLastInput(time_t	time){
	this->_lastInput = time;
}


bool					Client::getPing() const {return this->_ping;}
void					Client::setPing(bool b){
	this->_ping = b;
}


std::string&			Client::getInputBuf() {return this->_inputBuf;}

void					Client::appendInputBuf(const std::string& data) {
	this->_inputBuf += data;
}

void					Client::eraseFromInputBuf(size_t count) {
	this->_inputBuf.erase(0, count);
}
