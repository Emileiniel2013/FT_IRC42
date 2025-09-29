/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: temil-da <temil-da@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/14 19:03:44 by temil-da          #+#    #+#             */
/*   Updated: 2025/09/29 19:51:07 by temil-da         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <string>
#include <set>
#include <map>
#include "Channel.hpp"

class	Client{
private:
	int						_id;
	std::string				_nick;
	std::string				_user;
	std::string				_name;
	std::string				_hostname;
	std::set<std::string>	_channels;
	bool					_passOk = false;
	bool					_nickOk = false;
	bool					_userOk = false;
	bool					_authenticated = false;
	bool					_exit = false;
	std::string				_inputBuf;
public:
	Client();
	Client(int id);
	Client(const Client& other);
	Client&	operator=(const Client& other);
	~Client();

	// GETTERS/SETTERS
	int						getId() const;
	void					setId(int id);
	const std::string&		getNick() const;
	std::string				getReplyNick() const;
	std::string				getPrefix() const;
	void					setNick(const std::string& nick);
	const std::string&		getUser() const;
	void					setUser(const std::string& user);
	const std::string&		getName() const;
	void					setName(const std::string& name);
	void					setHostname(const std::string& host);
	std::set<std::string>	getChannels() const;
	void					addChannel(const std::string& channel);
	void					remChannel(const std::string& channel);
	void					setPassOk(bool b);
	bool					getPassOk() const;
	void					setNickOk(bool b);
	bool					getNickOk() const;	
	void					setUserOk(bool b);
	bool					getUserOk() const;
	bool					getAuth() const;
	void					setAuth(bool b);
	void					setExit(bool b);
	bool					getExit(void) const;
	std::string&			getInputBuf();
	void					appendInputBuf(const std::string& data);
	void					eraseFromInputBuf(size_t count);
};
