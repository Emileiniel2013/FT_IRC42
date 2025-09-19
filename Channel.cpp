/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: temil-da <temil-da@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/15 14:58:19 by temil-da          #+#    #+#             */
/*   Updated: 2025/09/19 19:45:00 by temil-da         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"

Channel::Channel(std::string name) : _name(name) {}

Channel::Channel(const Channel& other) {
	_name = other._name;
	_members = other._members;
	_operators = other._operators;
	_topic = other._topic;
	_inviteOnly = other._inviteOnly;
	_topicRestrict = other._topicRestrict;
	_pass = other._pass;
	_userLimit = other._userLimit;
}

Channel&	Channel::operator=(const Channel& other) {
	if (this != &other){
		_name = other._name;
		_members = other._members;
		_operators = other._operators;
		_topic = other._topic;
		_inviteOnly = other._inviteOnly;
		_topicRestrict = other._topicRestrict;
		_pass = other._pass;
		_userLimit = other._userLimit;
	}
	return *this;
}

Channel::~Channel() {}

//GETTERS/SETTERS
const std::string&	Channel::getName() const {return this->_name;}

void				Channel::addMember(int id) {
	this->_members.insert(id);
}

bool				Channel::isMember(int id) const {
	return this->_members.find(id) != this->_members.end();
}

void				Channel::removeMember(int id) {
	this->_members.erase(id);
}

void				Channel::addOperator(int id) {
	this->_operators.insert(id);
}

bool				Channel::isOperator(int id) const {
	return this->_operators.find(id) != this->_operators.end();
}

void				Channel::removeOperator(int id) {
	this->_operators.erase(id);
}

bool				Channel::getInviteOnly() const {return this->_inviteOnly;}

void				Channel::setInviteOnly(bool b) {this->_inviteOnly = b;}

void				Channel::addInvite(int id) {_invited.insert(id);}

bool				Channel::isInvited(int id) const {
	if (_invited.count(id) > 0)
		return true;
	return false;
}

void				Channel::rmInvite(int id) {_invited.erase(id);}

bool				Channel::getTopicRestrict() const {return this->_topicRestrict;}

void				Channel::setTopicRestrict(bool b) {this->_topicRestrict = b;}

void				Channel::setPass(const std::string& pass) {this->_pass = pass;}

const std::string&	Channel::getPass() const {return this->_pass;}

void				Channel::setUserLimit(int limit) {this->_userLimit = limit;}

int					Channel::getUserLimit() const {return this->_userLimit;}
