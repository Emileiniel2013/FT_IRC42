/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: temil-da <temil-da@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/17 22:55:59 by temil-da          #+#    #+#             */
/*   Updated: 2025/09/26 13:05:43 by temil-da         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Utils.hpp"
#include <sstream>

std::vector<std::string>	split(const std::string& str, char delim){
	std::vector<std::string>	res;
	std::string					currStr;
	std::istringstream			ss(str);
	while (std::getline(ss, currStr, delim)){
		if (!currStr.empty())
			res.push_back(currStr);
	}
	return res;
}

void						trimPrefix(std::string& str){
	if (!str.empty() && str[0] == ' ')
		str.erase(0, 1);
	if (!str.empty() && str[0] == ':')
		str.erase(0, 1);
}