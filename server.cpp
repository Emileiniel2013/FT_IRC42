// filepath: /home/silndoj/Core/IRC_42/server_commit1.cpp
// Commit 1: feat: upgrade to IPv6 socket with dual-stack support
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

int main()
{
    // creating socket (upgraded to IPv6)
    int serverSocket = socket(AF_INET6, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("socket");
        return 1;
    }

    // Set SO_REUSEADDR for quick server restart
    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("setsockopt SO_REUSEADDR");
        close(serverSocket);
        return 1;
    }

    // Disable IPV6_V6ONLY to accept IPv4-mapped clients
    opt = 0;
    if (setsockopt(serverSocket, IPPROTO_IPV6, IPV6_V6ONLY, &opt, sizeof(opt)) == -1) {
        perror("setsockopt IPV6_V6ONLY");
        close(serverSocket);
        return 1;
    }

    // specifying the address (upgraded to IPv6)
    sockaddr_in6 serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin6_family = AF_INET6;
    serverAddress.sin6_port = htons(8080);
    serverAddress.sin6_addr = in6addr_any;

    // binding socket
    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        perror("bind");
        close(serverSocket);
        return 1;
    }

    // listening to the assigned socket
    if (listen(serverSocket, 5) == -1) {
        perror("listen");
        close(serverSocket);
        return 1;
    }

    // accepting connection request
    int clientSocket = accept(serverSocket, nullptr, nullptr);
    if (clientSocket == -1) {
        perror("accept");
        close(serverSocket);
        return 1;
    }

    // receiving data
    char buffer[1024] = { 0 };
    ssize_t bytes_read = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytes_read > 0) {
        std::cout << "Message from client " << clientSocket << " : " << buffer << std::endl;
    }

    // closing the sockets
    close(clientSocket);
    close(serverSocket);

    return 0;
}