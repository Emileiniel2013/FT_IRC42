// filepath: /home/silndoj/Core/IRC_42/server_commit3.cpp
// Commit 3: feat: add poll()-based event loop architecture
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <vector>
#include <unordered_map>
#include <errno.h>

struct Client {
    std::string write_buf;
};

int main()
{
    // creating socket (IPv6)
    int listen_fd = socket(AF_INET6, SOCK_STREAM, 0);
    if (listen_fd == -1) {
        perror("socket");
        return 1;
    }

    // Set SO_REUSEADDR for quick server restart
    int opt = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("setsockopt SO_REUSEADDR");
        close(listen_fd);
        return 1;
    }

    // Disable IPV6_V6ONLY to accept IPv4-mapped clients
    opt = 0;
    if (setsockopt(listen_fd, IPPROTO_IPV6, IPV6_V6ONLY, &opt, sizeof(opt)) == -1) {
        perror("setsockopt IPV6_V6ONLY");
        close(listen_fd);
        return 1;
    }

    // specifying the address (IPv6)
    sockaddr_in6 serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin6_family = AF_INET6;
    serverAddress.sin6_port = htons(8080);
    serverAddress.sin6_addr = in6addr_any;

    // binding socket
    if (bind(listen_fd, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        perror("bind");
        close(listen_fd);
        return 1;
    }

    // listening to the assigned socket
    if (listen(listen_fd, 5) == -1) {
        perror("listen");
        close(listen_fd);
        return 1;
    }

    // Make the listening socket non-blocking
    int flags = fcntl(listen_fd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl F_GETFL");
        close(listen_fd);
        return 1;
    }
    if (fcntl(listen_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl F_SETFL");
        close(listen_fd);
        return 1;
    }

    // Create a pollfd list
    std::vector<pollfd> pollfds;
    pollfd listen_pfd = {listen_fd, POLLIN, 0};
    pollfds.push_back(listen_pfd);

    // Prepare per-client storage
    std::unordered_map<int, Client> clients;
    std::unordered_map<int, size_t> fd_to_index;
    fd_to_index[listen_fd] = 0;

    std::cout << "Server listening on port 8080 with poll()..." << std::endl;

    // Basic poll loop (simplified for this commit)
    for (int i = 0; i < 3; ++i) {  // Just 3 iterations for demo
        int poll_count = poll(pollfds.data(), pollfds.size(), 1000);  // 1 second timeout
        if (poll_count == -1) {
            perror("poll");
            break;
        } else if (poll_count == 0) {
            std::cout << "Poll timeout, no events" << std::endl;
            continue;
        }

        // Check events
        for (size_t j = 0; j < pollfds.size(); ++j) {
            if (pollfds[j].revents & POLLIN) {
                if (pollfds[j].fd == listen_fd) {
                    std::cout << "New connection available" << std::endl;
                    // Accept logic would go here in next commit
                } else {
                    std::cout << "Data available on client fd " << pollfds[j].fd << std::endl;
                    // Client data handling would go here in next commit
                }
            }
        }
    }

    // closing the socket
    close(listen_fd);

    return 0;
}