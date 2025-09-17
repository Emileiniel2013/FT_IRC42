// filepath: /home/silndoj/Core/IRC_42/server_commit4.cpp
// Commit 4: feat: implement multi-client echo server functionality
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <vector>
#include <unordered_map>
#include <string>
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

    std::cout << "Multi-client echo server listening on port 8080..." << std::endl;

    // Main loop with echo functionality
    while (true) {
        int poll_count = poll(pollfds.data(), pollfds.size(), -1);
        if (poll_count == -1) {
            perror("poll");
            break;
        }

        // Take a snapshot of returned pollfd entries
        std::vector<pollfd> snapshot = pollfds;

        for (size_t i = 0; i < snapshot.size(); ++i) {
            int fd = snapshot[i].fd;
            short revents = snapshot[i].revents;

            if (fd == listen_fd && (revents & POLLIN)) {
                // Accept new connections
                while (true) {
                    sockaddr_in6 clientAddr;
                    socklen_t clientLen = sizeof(clientAddr);
                    int client_fd = accept(listen_fd, (struct sockaddr*)&clientAddr, &clientLen);
                    
                    if (client_fd == -1) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            break; // No more connections to accept
                        }
                        perror("accept");
                        break;
                    }

                    // Set client socket non-blocking
                    int client_flags = fcntl(client_fd, F_GETFL, 0);
                    if (client_flags == -1 || fcntl(client_fd, F_SETFL, client_flags | O_NONBLOCK) == -1) {
                        perror("fcntl client non-blocking");
                        close(client_fd);
                        continue;
                    }

                    // Add to pollfd vector
                    pollfd client_pfd = {client_fd, POLLIN, 0};
                    pollfds.push_back(client_pfd);
                    fd_to_index[client_fd] = pollfds.size() - 1;

                    // Add to client map
                    clients[client_fd] = Client();

                    std::cout << "New client connected: " << client_fd << std::endl;
                }
            } else if (fd != listen_fd) {
                // Handle client events
                if (revents & POLLIN) {
                    // Read data from client
                    char buffer[1024];
                    while (true) {
                        ssize_t bytes_read = recv(fd, buffer, sizeof(buffer) - 1, 0);
                        
                        if (bytes_read > 0) {
                            buffer[bytes_read] = '\0';
                            clients[fd].write_buf += buffer;
                            
                            // Set POLLOUT to echo data back
                            size_t idx = fd_to_index[fd];
                            if (idx < pollfds.size()) {
                                pollfds[idx].events |= POLLOUT;
                            }
                        } else if (bytes_read == 0) {
                            // Client closed connection (basic handling)
                            std::cout << "Client " << fd << " disconnected" << std::endl;
                            close(fd);
                            // Note: Proper cleanup will be in next commit
                            goto next_iteration;
                        } else {
                            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                                break; // No more data to read
                            }
                            perror("recv");
                            close(fd);
                            goto next_iteration;
                        }
                    }
                }

                if (revents & POLLOUT) {
                    // Send data to client (echo functionality)
                    Client& client = clients[fd];
                    if (!client.write_buf.empty()) {
                        ssize_t bytes_sent = send(fd, client.write_buf.c_str(), client.write_buf.size(), 0);
                        
                        if (bytes_sent > 0) {
                            client.write_buf.erase(0, bytes_sent);
                            
                            // If buffer is empty, clear POLLOUT flag
                            if (client.write_buf.empty()) {
                                size_t idx = fd_to_index[fd];
                                if (idx < pollfds.size()) {
                                    pollfds[idx].events &= ~POLLOUT;
                                }
                            }
                        } else if (bytes_sent == -1) {
                            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                                perror("send");
                                close(fd);
                            }
                        }
                    }
                }
            }
        }
        next_iteration:;
    }

    // closing the socket
    close(listen_fd);

    return 0;
}