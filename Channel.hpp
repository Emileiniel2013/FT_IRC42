/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: temil-da <temil-da@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/15 14:58:29 by temil-da          #+#    #+#             */
/*   Updated: 2025/09/19 19:42:02 by temil-da         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <string>
#include <set>

class	Channel{
private:
	std::string		_name;
	std::set<int>	_members;
	std::set<int>	_operators;
	std::string		_topic;
	bool			_inviteOnly = false;
	std::set<int>	_invited;
	bool			_topicRestrict = false;
	std::string		_pass;
	int				_userLimit = 0;
public:
	Channel(std::string	name);
	Channel(const Channel& other);
	Channel&	operator=(const Channel& other);
	~Channel();

	// GETTERS/SETTERS
	const std::string&	getName() const;
	void				addMember(int id);
	bool				isMember(int id) const;
	void				removeMember(int id);
	void				addOperator(int id);
	bool				isOperator(int id) const;
	void				removeOperator(int id);
	bool				getInviteOnly() const;
	void				setInviteOnly(bool b);
	void				addInvite(int id);
	bool				isInvited(int id) const;
	void				rmInvite(int id);
	bool				getTopicRestrict() const;
	void				setTopicRestrict(bool b);
	void				setPass(const std::string& pass);
	const std::string&	getPass() const;
	void				setUserLimit(int limit);
	int					getUserLimit() const;
};
