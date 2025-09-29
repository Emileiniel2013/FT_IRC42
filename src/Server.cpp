/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: temil-da <temil-da@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/15 17:02:14 by temil-da          #+#    #+#             */
/*   Updated: 2025/09/29 19:59:17 by temil-da         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "ServerHelpers.hpp"
#include "ErrorCodes.hpp"
#include <sstream>
#include <vector>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <cstring>
#include <iostream>
#include <errno.h>

Server::Server(std::string name, std::string pass, int port) : _serverName(name), _serverPass(pass), _port(port), _listenFd(-1) {
	this->_cmdMap["PASS"] = &Server::handlePass;
	this->_cmdMap["NICK"] = &Server::handleNick;
	this->_cmdMap["USER"] = &Server::handleUser;
	this->_cmdMap["JOIN"] = &Server::handleJoin;
	this->_cmdMap["PRIVMSG"] = &Server::handlePrivmsg;
	this->_cmdMap["PING"] = &Server::handlePing;
	this->_cmdMap["PART"] = &Server::handlePart;
	this->_cmdMap["QUIT"] = &Server::handleQuit;
	this->_cmdMap["KICK"] = &Server::handleKick;
	this->_cmdMap["INVITE"] = &Server::handleInvite;
	this->_cmdMap["TOPIC"] = &Server::handleTopic;
	this->_cmdMap["MODE"] = &Server::handleMode;
}

Server::Server(const Server& other) : _cmdMap(other._cmdMap), _serverName(other._serverName), _serverPass(other._serverPass), _port(other._port), _listenFd(other._listenFd), _pollfds(other._pollfds), _fdToIndex(other._fdToIndex) {}

Server&	Server::operator=(const Server& other) {
	if (this != &other){
		this->_serverName = other._serverName;
		this->_serverPass = other._serverPass;
		this->_cmdMap = other._cmdMap;
	}
	return *this;
}

Server::~Server() {}

void	Server::scheduleFdClose(int fd){
	if (fd == _listenFd)
		return; // never schedule listening socket here
	_fdsToClose.insert(fd);
}

void	Server::sendError(Client& client, int errCode, const std::string& target, const std::string& text){
	std::ostringstream oss;
	oss << ":" << this->_serverName << " " << errCode << " "
		<< client.getReplyNick();
	if (!target.empty())
		oss << " " << target;
	oss << " :" << text << "\r\n";
	std::string	msg = oss.str();
	std::cout << "Message to client " + std::to_string(client.getId()) + ": " + msg + "\n";
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
		+ client.getPrefix() + "\r\n";
	std::cout << "Message to client " + std::to_string(client.getId()) + ": " + msg + "\n";
	send(client.getId(), msg.c_str(), msg.size(), 0);
}


void	Server::processInput(Client& client, std::string& buf) 
{
	std::string			cmd;
	std::istringstream	keyword(buf);

	keyword >> cmd;
	auto	it = this->_cmdMap.find(cmd);
	if (it == this->_cmdMap.end()){
		sendError(client, ERR_UNKNOWNCOMMAND, cmd, "Unknown command");
		return ;
	}
	if (cmd != "NICK" && cmd != "PASS" && cmd != "USER" && !client.getAuth()){
		sendError(client, ERR_NOTREGISTERED, "", "You have not registered");
		return ;
	}
	(this->*(it->second))(client, keyword);
}

void	Server::setupSocket(){
	// Create listening socket (IPv6)
	_listenFd = socket(AF_INET6, SOCK_STREAM, 0);
	if (_listenFd == -1) {
		throw std::runtime_error("Failed to create socket");
	}

	// Set SO_REUSEADDR
	int opt = 1;
	if (setsockopt(_listenFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
		close(_listenFd);
		throw std::runtime_error("Failed to set SO_REUSEADDR");
	}

	// Disable IPV6_V6ONLY to accept IPv4-mapped clients
	opt = 0;
	if (setsockopt(_listenFd, IPPROTO_IPV6, IPV6_V6ONLY, &opt, sizeof(opt)) == -1) {
		close(_listenFd);
		throw std::runtime_error("Failed to set IPV6_V6ONLY");
	}

	// Bind to in6addr_any
	sockaddr_in6 serverAddress;
	memset(&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin6_family = AF_INET6;
	serverAddress.sin6_port = htons(_port);
	serverAddress.sin6_addr = in6addr_any;

	if (bind(_listenFd, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
		close(_listenFd);
		throw std::runtime_error("Failed to bind socket");
	}

	// Listen
	if (listen(_listenFd, SOMAXCONN) == -1) {
		close(_listenFd);
		throw std::runtime_error("Failed to listen on socket");
	}

	// Make the listening socket non-blocking
	int flags = fcntl(_listenFd, F_GETFL, 0);
	if (flags == -1 || fcntl(_listenFd, F_SETFL, flags | O_NONBLOCK) == -1) {
		close(_listenFd);
		throw std::runtime_error("Failed to set socket non-blocking");
	}

	// Initialize pollfd list
	pollfd listenPfd = {_listenFd, POLLIN, 0};
	_pollfds.push_back(listenPfd);
	_fdToIndex[_listenFd] = 0;
}

void	Server::startServer(){
	setupSocket();
	std::cout << "Server listening on port " << _port << "..." << std::endl;

	// Main loop
	while (true) {
		int pollCount = poll(_pollfds.data(), _pollfds.size(), -1);
		if (pollCount == -1) {
			perror("poll");
			break;
		}

		// Take a snapshot of current pollfds
		std::vector<pollfd> snapshot = _pollfds;
		std::vector<int> fdsToClose;

		for (size_t i = 0; i < snapshot.size(); ++i) {
			int fd = snapshot[i].fd;
			short revents = snapshot[i].revents;

			if (fd == _listenFd && (revents & POLLIN)) {
				acceptNewClients();
			} else if (fd != _listenFd) {
				if (revents & POLLIN) {
					handleClientInput(fd, i);
				}
				if (revents & POLLOUT) {
					handleClientOutput(fd, i);
				}
				if (revents & (POLLERR | POLLHUP)) {
					std::cout << "Client " << fd << " error/hangup" << std::endl;
					fdsToClose.push_back(fd);
				}
			}
		}

		// Merge local vector into global set
		for (int fd : fdsToClose)
			_fdsToClose.insert(fd);

		// Process and clear global close set
		if (!_fdsToClose.empty()) {
			// copy to avoid iterator invalidation during cleanup
			std::vector<int> toClose(_fdsToClose.begin(), _fdsToClose.end());
			_fdsToClose.clear();
			for (int fd : toClose) {
				if (_clients.count(fd))
					cleanupClient(fd);
				else
					close(fd); // stray fd still open
			}
		}
	}

	// Cleanup on shutdown
	close(_listenFd);
	for (auto& pair : _clients) {
		close(pair.first);
	}
}

void	Server::acceptNewClients(){
	while (true) {
		sockaddr_in6 clientAddr;
		socklen_t clientLen = sizeof(clientAddr);
		int clientFd = accept(_listenFd, (struct sockaddr*)&clientAddr, &clientLen);
		
		if (clientFd == -1) {
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break; // No more connections to accept
			perror("accept");
			break;
		}
		
		// Set client socket non-blocking
		int clientFlags = fcntl(clientFd, F_GETFL, 0);
		if (clientFlags == -1 || fcntl(clientFd, F_SETFL, clientFlags | O_NONBLOCK) == -1) {
			perror("fcntl client non-blocking");
			close(clientFd);
			continue;
		}

		// Store client hostname (in this case it will most be default but whatever)
		char	host_str[NI_MAXHOST];
		std::string	clientHost = "unknown";
		if (getnameinfo((struct sockaddr*)&clientAddr, clientLen, host_str, 
				sizeof(host_str), NULL, 0, NI_NUMERICHOST) == 0)
			clientHost = host_str;

		// Add to pollfd vector
		pollfd clientPfd = {clientFd, POLLIN, 0};
		_pollfds.push_back(clientPfd);
		_fdToIndex[clientFd] = _pollfds.size() - 1;

		// Create new Client object
		_clients[clientFd] = Client(clientFd);
		_clients[clientFd].setHostname(clientHost);

		std::cout << "New client connected: " << clientFd << std::endl;
	}
}

void	Server::handleClientInput(int fd, size_t index){
	(void)index;
	std::vector<int> fdsToClose;
	
	while (!_clients[fd].getExit()) {
		char buffer[1024];
		ssize_t bytesRead = recv(fd, buffer, sizeof(buffer) - 1, 0);

		if (bytesRead > 0) {
			buffer[bytesRead] = '\0';
			_clients[fd].appendInputBuf(std::string(buffer, bytesRead));

			// Process all complete messages in the buffer
			std::string& inputBuf = _clients[fd].getInputBuf();
			size_t pos;
			while ((pos = inputBuf.find("\r\n")) != std::string::npos) {
				std::string message = inputBuf.substr(0, pos);
				inputBuf.erase(0, pos + 2);
				
				std::cout << "Message from client " << fd << ": " << message << std::endl;
				
				// Process the IRC command using existing handler
				processInput(_clients[fd], message);
				if (_clients[fd].getExit())
					break ;
			}
		} else if (bytesRead == 0) {
			// Client closed connection
			std::cout << "Client " << fd << " disconnected" << std::endl;
			fdsToClose.push_back(fd);
			break;
		} else {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				break; // No more data to read
			}
			perror("recv");
			fdsToClose.push_back(fd);
			break;
		}
	}
	
	// Defer actual cleanup to main loop
	for (int fdToClose : fdsToClose) {
		if (fdToClose != _listenFd)
			_fdsToClose.insert(fdToClose);
	}
}

void	Server::handleClientOutput(int fd, size_t index){
	(void)fd;
	(void)index;
	// This method can be used later for buffered output if needed
	// For now, we use direct send() calls in command handlers
}

void	Server::cleanupClient(int fd){
	close(fd);
	
	// Handle channel cleanup if client was authenticated
	if (_clients.count(fd) != 0) {
		Client& client = _clients[fd];
		if (client.getAuth()) {
			std::set<std::string> channels = client.getChannels();
			for (auto it = channels.begin(); it != channels.end(); ++it) {
				std::string reason = "Connection error";
				if (_quitReasons.count(fd))
					reason = _quitReasons[fd];
				std::string msg = ":" + client.getPrefix() + " QUIT :" + reason + "\r\n";
				std::string chName = *it;
				if (_channels.count(chName) == 0)
					continue;
				Channel& ch = _channels[chName];
				ch.removeMember(client.getId());
				ch.removeOperator(client.getId());
				ch.broadcast(msg);
				if (ch.getAllMembers().empty())
					_channels.erase(chName);
			}
		}
	}
	_quitReasons.erase(fd);
	
	_clients.erase(fd);
	size_t idx = _fdToIndex[fd];
	_fdToIndex.erase(fd);
	
	// Remove from pollfds vector
	if (idx < _pollfds.size()) {
		_pollfds.erase(_pollfds.begin() + idx);
		
		// Update fd_to_index mapping for shifted elements
		for (size_t i = idx; i < _pollfds.size(); ++i) {
			_fdToIndex[_pollfds[i].fd] = i;
		}
	}
}

void	Server::sendBroadcast(int senderFd, const std::string& message){
	std::string broadcastMsg = "Message from client " + std::to_string(senderFd) + ": " + message + "\r\n";
	
	for (auto &[otherFd, otherClient] : _clients) {
		if (otherFd == senderFd)
			continue; // skip the sender

		send(otherFd, broadcastMsg.c_str(), broadcastMsg.size(), 0);
	}
}