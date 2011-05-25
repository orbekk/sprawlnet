#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "string_util.h"
#include "socket_server.h"

const uint16_t PORT = 3000;
const int TIMEOUT = 30;

void error(const char *msg) {
    perror(msg);
    exit(1);
}

void enable_reuseaddr(int socket) {
    int value = 1;
    int status = setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(int));
    if (status == -1) {
        error("Failed to set SO_REUSEADDR on socket.");
    }
}

#if 0
int main() {
    int status;

    int listener_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (listener_socket == -1) {
        error("Could not create listening socket.");
    }
    
    enable_reuseaddr(listener_socket);

    struct sockaddr_in bind_address;
    bind_address.sin_family = AF_INET;
    bind_address.sin_port = htons(PORT);
    status = inet_pton(AF_INET, "0.0.0.0", &bind_address.sin_addr);
    if (status == 0) {
        error("Invalid address.");
    } else if (status == -1) {
        error("Unknown adress family");
    }

    status = bind(listener_socket, (struct sockaddr*)&bind_address, sizeof(bind_address));
    if (status == -1) {
        error("Could not bind to socket");
    }

    const int connection_queue = 10;
    status = listen(listener_socket, connection_queue);
    if (status == -1) {
        error("Could not set socket as a passive socket.");
    }
    
    int fdmax = listener_socket;
    fd_set sockets;
    FD_ZERO(&sockets);
    FD_SET(listener_socket, &sockets);

    while (1) {
        fd_set sockets_copy = sockets;

        struct timeval timeout = { 1, 0 };
        status = select(fdmax+1, &sockets_copy, NULL, NULL, &timeout);
        if (status == -1) {
            error("Select() failed.");
        }

        for (int i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &sockets_copy)) {
                if (listener_socket == i) {
                    struct sockaddr_in client_address;
                    socklen_t client_address_size = sizeof(client_address);
                    int client_socket = accept(listener_socket,
                            (struct sockaddr*)&client_address,
                            &client_address_size);
                    if (client_socket >= 0) {
                        char client_address_str[INET_ADDRSTRLEN];                        
                        char *client_address_str_tmp = inet_ntoa(client_address.sin_addr);
                        strcpy(client_address_str, client_address_str_tmp);

                        printf("Received connection from %s\n",
                                client_address_str);
                        FD_SET(client_socket, &sockets);
                        if (client_socket > fdmax) {
                            fdmax = client_socket;
                        }
                    } else {
                        perror("Could not accept connection.");
                    }
                } else {
                    const size_t BUFSIZE = 1024;
                    char buffer[BUFSIZE+1];
                    int num_bytes_read = recv(i, buffer, BUFSIZE, 0);
                    if (num_bytes_read < 0) {
                        perror("Could not receive from socket: recv()");
                        close(i);
                        FD_CLR(i, &sockets);
                    } else if (num_bytes_read == 0) {
                        printf("Connection closed.\n");
                        close(i);
                        FD_CLR(i, &sockets);
                    } else {
                        buffer[num_bytes_read] = '\0';
                        printf("Received data from client: %s\n", buffer); 
                    }
                }
            }
        }
    }
}
#endif

namespace omninet {

SocketServer::ConnectionManager* SocketServer::ConnectionManager::create() {
    ConnectionManager* manager = new ConnectionManager();
    manager->init();
    return manager;
}

std::string SocketServer::Connection::get_address_str() const {
    std::string address_str = inet_ntoa(address.sin_addr);

    address_str += ":";
    int port = ntohs(address.sin_port);
    address_str += int_to_string(port);

    return address_str;
}

SocketServer::ConnectionManager::~ConnectionManager() {
    // TODO: Implement this.
}

void SocketServer::ConnectionManager::init() {
    FD_ZERO(&connection_fds);
}

void SocketServer::ConnectionManager::get_connections_fds(fd_set *dest) const {
    *dest = connection_fds;
}

void SocketServer::ConnectionManager::add_connection(Connection *connection) {
    add_fd(connection->get_fd());
    active_connections[connection->get_fd()] = shared_ptr<Connection>(connection);
}

void SocketServer::ConnectionManager::remove_connection(
        const Connection *connection) {
    remove_connection_by_fd(connection->get_fd());
}

void SocketServer::ConnectionManager::remove_connection_by_fd(int fd) {
    map<int, shared_ptr<Connection> >::iterator it;
    it = active_connections.find(fd);
    if (it != active_connections.end()) {
        remove_fd(fd);
        active_connections.erase(it);
    }
}

void SocketServer::ConnectionManager::add_fd(int fd) {
    if (fd > fdmax) {
        fdmax = fd;
    }
    FD_SET(fd, &connection_fds);
}

void SocketServer::ConnectionManager::remove_fd(int fd) {
    // TODO: Do we have to update fdmax? I'm guessing no.
    FD_CLR(fd, &connection_fds);
}

bool SocketServer::ConnectionManager::get_connection(int fd,
        Connection *dest) const {
    map<int, shared_ptr<Connection> >::const_iterator it;
    it = active_connections.find(fd);
    if (it != active_connections.end()) {
        it->second->copy_to(dest);
        return true;
    } else {
        return false;
    }
}

void SocketServer::init() {
    connection_manager.reset(new ConnectionManager());
}

int SocketServer::bind(const char *port) {
    int status;
    int sockets_bound = 0;
    struct addrinfo hints;
    init_hints(&hints);

    struct addrinfo *result, *rp;
    status = getaddrinfo(NULL, port, &hints, &result);

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        int fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (fd == -1) {
            perror("Could not create socket.");
        } else {
            status = ::bind(fd, rp->ai_addr, rp->ai_addrlen);
            if (status == -1) {
                perror("Could not bind to socket.");
            } else {
                printf("Bound to socket.\n");
                sockets_bound++;
                // TODO: Register fds.
            }
        }
    }

    freeaddrinfo(result);
    return sockets_bound;
}

void SocketServer::init_hints(struct addrinfo *hints) {
    memset(hints, 0, sizeof(struct addrinfo));
    hints->ai_family = AF_UNSPEC;
    hints->ai_socktype = SOCK_DGRAM;
    hints->ai_flags = AI_PASSIVE;
    hints->ai_protocol = 0;
    hints->ai_canonname = NULL;
    hints->ai_addr = NULL;
    hints->ai_next = NULL;
}

}
