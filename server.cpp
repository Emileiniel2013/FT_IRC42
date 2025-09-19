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
    std::string _message;
};

// void    parse_str(std::string)

int main()
{
    // 1) Create listening socket (IPv6)
    int listen_fd = socket(AF_INET6, SOCK_STREAM, 0);
    if (listen_fd == -1) {
        perror("socket");
        return 1;
    }

    // Set SO_REUSEADDR
    int opt = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("setsockopt SO_REUSEADDR");
        close(listen_fd);
        return 1;
    }

    // Disable IPV6_V6ONLY to accept IPv4-mapped clients(opt 0 = disable)
    opt = 0;
    if (setsockopt(listen_fd, IPPROTO_IPV6, IPV6_V6ONLY, &opt, sizeof(opt)) == -1) {
        perror("setsockopt IPV6_V6ONLY");
        close(listen_fd);
        return 1;
    }

    // Bind to in6addr_any(listen every interface)
    sockaddr_in6 serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin6_family = AF_INET6;
    serverAddress.sin6_port = htons(6676);
    serverAddress.sin6_addr = in6addr_any;

    if (bind(listen_fd, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        perror("bind");
        close(listen_fd);
        return 1;
    }

    // Listen && SOMAXCONN its the core max capacity
    if (listen(listen_fd, SOMAXCONN) == -1) {
        perror("listen");
        close(listen_fd);
        return 1;
    }

    // 2) Make the listening socket non-blocking
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

    // 3) Create a pollfd list
    std::vector<pollfd> pollfds;
    pollfd listen_pfd = {listen_fd, POLLIN, 0};
    pollfds.push_back(listen_pfd);

    // 4) Prepare per-client storage
    std::unordered_map<int, Client> clients;
    std::unordered_map<int, size_t> fd_to_index;
    fd_to_index[listen_fd] = 0;

    std::cout << "Server listening on port 8080..." << std::endl;

    // 5) Main loop
    while (true) {
        int poll_count = poll(pollfds.data(), pollfds.size(), -1);
        if (poll_count == -1) {
            perror("poll");
            break;
        }

        // Take a snapshot of returned pollfd entries
        std::vector<pollfd> snapshot = pollfds;
        std::vector<int> fds_to_close;

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
            } 
            
            else if (fd != listen_fd) {
                // Handle client events
                if (revents & POLLIN) {
                    // Read data from client
                    while (true)
                    {
                        char buffer[1024];
                        ssize_t bytes_read = recv(fd, buffer, sizeof(buffer) - 1, 0);

                        if (bytes_read > 0)
                        {
                            buffer[bytes_read] = '\0'; // make it C-string safe
                            clients[fd].write_buf.append(buffer, bytes_read);

                            size_t pos;
                            // Process all complete messages in the buffer
                            while ((pos = clients[fd].write_buf.find("\r\n")) != std::string::npos)
                            {
                                std::string message = clients[fd].write_buf.substr(0, pos);
                                clients[fd].write_buf.erase(0, pos + 2);
                                clients[fd]._message = message;
                                // Display received message
                                std::cout << "Message from client " << fd << ": " << clients[fd]._message << std::endl;
                            }
                        }
                        else if (bytes_read == 0)
                        {
                            // Client closed connection
                            std::cout << "Client " << fd << " disconnected" << std::endl;
                            fds_to_close.push_back(fd);
                            break;
                        }
                        else
                        {
                            if (errno == EAGAIN || errno == EWOULDBLOCK)
                            {
                                break; // No more data to read
                            }
                            perror("recv");
                            fds_to_close.push_back(fd);
                            break;
                        }
                    }
                }

                if (revents & POLLOUT) {
                    // Send data to client
                    Client& client = clients[fd];
                    if (!client._message.empty()) {
                        ssize_t bytes_sent = send(fd, client._message.c_str(), client._message.size(), 0);
                        
                        if (bytes_sent > 0) {
                            client._message.erase(0, bytes_sent);
                            
                            // If buffer is empty, clear POLLOUT flag
                            if (client._message.empty()) {
                                size_t idx = fd_to_index[fd];
                                if (idx < pollfds.size()) {
                                    pollfds[idx].events &= ~POLLOUT;
                                }
                            }
                        } else if (bytes_sent == -1) {
                            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                                perror("send");
                                fds_to_close.push_back(fd);
                            }
                        }
                    }
                }

                if (revents & (POLLERR | POLLHUP)) {
                    // Error or hangup
                    std::cout << "Client " << fd << " error/hangup" << std::endl;
                    fds_to_close.push_back(fd);
                }
            }
        }

        // Close scheduled file descriptors
        for (int fd : fds_to_close) {
            close(fd);
            clients.erase(fd);
            
            size_t idx = fd_to_index[fd];
            fd_to_index.erase(fd);
            
            // Remove from pollfds vector
            if (idx < pollfds.size()) {
                pollfds.erase(pollfds.begin() + idx);
                
                // Update fd_to_index mapping for shifted elements
                for (size_t i = idx; i < pollfds.size(); ++i) {
                    fd_to_index[pollfds[i].fd] = i;
                }
            }
        }
    }

    // 6) Close resources on server shutdown
    close(listen_fd);
    for (auto& pair : clients) {
        close(pair.first);
    }

    return 0;
}