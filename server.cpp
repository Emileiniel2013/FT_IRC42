// filepath: /home/silndoj/Core/IRC_42/server_commit2.cpp
// Commit 2: feat: implement non-blocking I/O for scalable connections
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

int main()
{
    // creating socket (IPv6)
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

    // specifying the address (IPv6)
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

    // Make the listening socket non-blocking
    int flags = fcntl(serverSocket, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl F_GETFL");
        close(serverSocket);
        return 1;
    }
    if (fcntl(serverSocket, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl F_SETFL");
        close(serverSocket);
        return 1;
    }

    std::cout << "Server listening on port 8080 (non-blocking)..." << std::endl;

    // accepting connection request (now non-blocking)
    int clientSocket = accept(serverSocket, nullptr, nullptr);
    if (clientSocket == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            std::cout << "No connection pending" << std::endl;
        } else {
            perror("accept");
        }
        close(serverSocket);
        return 1;
    }

    // Set client socket non-blocking too
    int client_flags = fcntl(clientSocket, F_GETFL, 0);
    if (client_flags == -1 || fcntl(clientSocket, F_SETFL, client_flags | O_NONBLOCK) == -1) {
        perror("fcntl client non-blocking");
        close(clientSocket);
        close(serverSocket);
        return 1;
    }

    // receiving data (now non-blocking)
    char buffer[1024] = { 0 };
    ssize_t bytes_read = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytes_read > 0) {
        std::cout << "Message from client " << clientSocket << " : " << buffer << std::endl;
    } else if (bytes_read == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            std::cout << "No data available" << std::endl;
        } else {
            perror("recv");
        }
    }

    // closing the sockets
    close(clientSocket);
    close(serverSocket);

    return 0;
}